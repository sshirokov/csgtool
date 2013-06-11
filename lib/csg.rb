require 'ffi'

module CSG
  module Native
    extend FFI::Library
    ffi_lib 'csg'

    class STLObject < FFI::ManagedStruct
      layout :header, [:uint8, 80],
             :facet_count, :uint32,
             :stl_facet, :pointer

      def self.release(ptr)
        CSG::Native.stl_free ptr
      end
    end

    class BSPNode < FFI::ManagedStruct
      layout :polygons, :pointer,
             :divider, :pointer,
             :front, :pointer,
             :back,  :pointer

      def self.release(ptr)
        CSG::Native.free_bsp_tree ptr
      end
    end

    attach_function :stl_read_file, [:string, :bool], :pointer
    attach_function :stl_write_file, [:pointer, :string], :int

    attach_function :stl_free, [:pointer], :void
    attach_function :free_bsp_tree, [:pointer], :void

    attach_function :stl_to_bsp, [:pointer], :pointer
    attach_function :bsp_to_stl, [:pointer], :pointer

    attach_function :bsp_union, [:pointer, :pointer], :pointer
    attach_function :bsp_subtract, [:pointer, :pointer], :pointer
    attach_function :bsp_intersect, [:pointer, :pointer], :pointer
  end

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
