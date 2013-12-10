require 'ffi'

module CSG
  module Native
    extend FFI::Library

    candidates = ['.bundle', '.so', '.dylib', ''].map do |ext|
      File.join(File.expand_path("../..", __FILE__), 'libcsg' + ext)
    end
    ffi_lib ['csg'] + candidates

    class Mesh < FFI::ManagedStruct
      # WARNING: You should probably never call `:destroy`, because it's a managed
      #          struct, and should get destroyed by Ruby using this very method
      #          inside of `Mesh.release`
      layout :type, [:uint8, 4],
             :init,        callback([:pointer, :pointer], :int),
             :destroy,     callback([:pointer], :void),
             :poly_count,  callback([:pointer], :int),
             :to_polygons, callback([:pointer], :pointer),
             :to_bsp,      callback([:pointer], :pointer),
             :write,       callback([:pointer, :string], :int)

      def self.release(ptr)
        CSG::Native.destroy_mesh ptr
      end
    end

    attach_function :destroy_mesh, [:pointer], :void
    attach_function :mesh_read_file, [:string], Mesh

    class BSPNode < FFI::ManagedStruct
      layout :polygons, :pointer,
             :divider, :pointer,
             :front, :pointer,
             :back,  :pointer

      def self.release(ptr)
        CSG::Native.free_bsp_tree ptr
      end
    end

    attach_function :free_bsp_tree, [:pointer], :void
    attach_function :mesh_to_bsp, [Mesh], BSPNode

    attach_function :bsp_union, [:pointer, :pointer], :pointer
    attach_function :bsp_subtract, [:pointer, :pointer], :pointer
    attach_function :bsp_intersect, [:pointer, :pointer], :pointer
  end
end

module CSG
  class Solid
    attr_reader :mesh

    def initialize(opts)
      if opts[:file]
        load_from_file opts[:file]
      elsif opts[:mesh]
        @mesh = opts[:mesh]
      end
      raise ArgumentError.new "Failed to load mesh with: #{opts.inspect}" unless @mesh
    end

    def load_from_file(path)
      File.stat(path) # Stat before load to raise a sane "Does not exist" error
      mesh_ptr = CSG::Native::mesh_read_file path
      if not mesh_ptr.null?
          @mesh = CSG::Native::Mesh.new mesh_ptr
      else
        raise Exception.new("Failed to produce Mesh from #{path}")
      end
    end

    [:intersect, :subtract, :union].each do |name|
      define_method name do |solid|
        # I'm so paranoid because ruby will gladly FFI through a
        # *NULL and explode if you ask, and that's much worse.
        raise Exception.new("The calling mesh is a NULL pointer") if mesh.null?
        raise Exception.new("The parameter mesh is a NULL pointer") if solid.mesh.null?
        raise Exception.new("My BSP tree is NULL.") if (my_bsp_ptr = mesh[:to_bsp].call mesh).null?
        raise Exception.new("My BSP tree is NULL.") if (their_bsp_ptr = solid.mesh[:to_bsp].call solid.mesh).null?
        my_bsp = CSG::Native::BSPNode.new(my_bsp_ptr)
        their_bsp = CSG::Native::BSPNode.new(their_bsp_ptr)

        result_ptr = CSG::Native.send "bsp_#{name}", my_bsp, their_bsp
        raise Exception.new("Result of #{name} is NULL") if result_ptr.null?

        # We will not wrap te result in a CSG::Native::BSPNode because
        # to avoid garbage collection, and we'll manage this pointer
        # inside of the CSG::Native::Mesh object we get with
        # bsp_to_mesh(.., 0) - which will not clone the input parameter
        raise Exception.new("TODO: Make a BSP-backed mesh and return it.")
      end
    end

  end
end
