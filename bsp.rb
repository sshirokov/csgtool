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



object = CSG::Native::STLObject.new( CSG::Native.stl_read_file(ARGV[0], true) )
object2 = CSG::Native::STLObject.new( CSG::Native.stl_read_file(ARGV[1], true) )

object_bsp = CSG::Native.stl_to_bsp(object)
object2_bsp = CSG::Native.stl_to_bsp(object2)

intersect_bsp = CSG::Native.bsp_intersect(object_bsp, object2_bsp);
substract_bsp = CSG::Native.bsp_subtract(object_bsp, object2_bsp);
union_bsp = CSG::Native.bsp_union(object_bsp, object2_bsp);

CSG::Native.free_bsp_tree object_bsp
CSG::Native.free_bsp_tree object2_bsp
object_bsp = object2_bsp = nil

i = CSG::Native.bsp_to_stl(intersect_bsp)
s = CSG::Native.bsp_to_stl(substract_bsp)
u = CSG::Native.bsp_to_stl(union_bsp)

CSG::Native.stl_write_file(u, "union.stl")
CSG::Native.stl_write_file(s, "substract.stl")
CSG::Native.stl_write_file(i, "intersect.stl")
