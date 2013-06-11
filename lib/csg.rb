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
end
