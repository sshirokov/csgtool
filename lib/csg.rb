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
    attr_reader :tree

    def initialize(opts)
      if opts[:file]
        load_from_file opts[:file]
      elsif opts[:tree]
        @tree = opts[:tree]
      end
      raise ArgumentError.new "Failed to load tree with: #{opts.inspect}" unless @tree
    end

    def load_from_file(path)
      File.stat(path) # Stat before load to raise a sane "Does not exist" error
      mesh_ptr = CSG::Native::mesh_read_file path
      if not mesh_ptr.null?
        if (tree_ptr = CSG::Native.mesh_to_bsp mesh_ptr)
          @tree = CSG::Native::BSPNode.new tree_ptr
        else
          raise Exception.new("Failed to convert Mesh into BSPNode")
        end
      else
        raise Exception.new("Failed to produce Mesh from #{path}")
      end
    end

    [:intersect, :subtract, :union].each do |name|
      define_method name do |solid|
        ptr = CSG::Native.send "bsp_#{name}", tree, solid.tree
        tree = CSG::Native::BSPNode.new(ptr)
        CSG::Solid.new :tree => tree
      end
    end

  end
end

# TODO: most of this should go away and get replaced
#       with the CSG::Native::Mesh based stuff
module CSG
  module Native
    class STLObject < FFI::ManagedStruct
      layout :header, [:uint8, 80],
             :facet_count, :uint32,
             :stl_facet, :pointer

      def self.release(ptr)
        CSG::Native.stl_free ptr
      end

      def write_file(path)
        CSG::Native.stl_write_file self, path
      end
    end



    attach_function :stl_read_file, [:string, :bool], :pointer
    attach_function :stl_write_file, [:pointer, :string], :int

    attach_function :stl_free, [:pointer], :void

    attach_function :stl_to_bsp, [:pointer], :pointer
    attach_function :bsp_to_stl, [:pointer], :pointer
  end

  class OldSolid
    attr_reader :tree

    def initialize(opts)
      if opts[:file]
        load_from_file opts[:file]
      elsif opts[:tree]
        @tree = opts[:tree]
      end
      raise ArgumentError.new "Failed to load tree with: #{opts.inspect}" unless @tree
    end

    def load_from_file(path)
      File.stat(path) # Stat before load to raise a sane "Does not exist" error
      stl = CSG::Native::STLObject.new CSG::Native.stl_read_file(path, false)
      @tree = CSG::Native::BSPNode.new CSG::Native.stl_to_bsp(stl)
    end

    def to_stl
      ptr = CSG::Native.bsp_to_stl tree
      CSG::Native::STLObject.new(ptr)
    end

    # Build the CSG methods with ManagedStruct wrappers
    [:intersect, :subtract, :union].each do |name|
      define_method name do |solid|
        ptr = CSG::Native.send "bsp_#{name}", tree, solid.tree
        tree = CSG::Native::BSPNode.new(ptr)
        CSG::Solid.new :tree => tree
      end
    end
  end
end
