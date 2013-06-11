require 'ffi'

module CSG
  module Native
    extend FFI::Library
    ffi_lib 'csg'

    FLOAT3 = FFI::ArrayType.new(FFI::Type::FLOAT, 3)
    typedef FLOAT3, :float3

    class STLFacet < FFI::Struct
      layout :normal, :float3,
      :vertices, FFI::ArrayType.new(CSG::Native::FLOAT3, 3),
      :attr, :uint16
    end

    class STLObject < FFI::Struct
      layout :header, [FFI::Type::UINT8, 80],
      :facet_count, :uint32,
      :stl_facet, :pointer
    end

    attach_function :stl_read_file, [:string, :bool], :pointer
    attach_function :stl_write_file, [:pointer, :string], :int

    attach_function :stl_to_bsp, [:pointer], :pointer
    attach_function :bsp_to_stl, [:pointer], :pointer

    attach_function :bsp_union, [:pointer, :pointer], :pointer
    attach_function :bsp_subtract, [:pointer, :pointer], :pointer
    attach_function :bsp_intersect, [:pointer, :pointer], :pointer
  end
end



object = CSG::Native.stl_read_file(ARGV[0], true)
object2 = CSG::Native.stl_read_file(ARGV[1], true)

object_bsp = CSG::Native.stl_to_bsp(object)
object2_bsp = CSG::Native.stl_to_bsp(object2)

intersect_bsp = CSG::Native.bsp_intersect(object_bsp, object2_bsp);
substract_bsp = CSG::Native.bsp_subtract(object_bsp, object2_bsp);
union_bsp = CSG::Native.bsp_union(object_bsp, object2_bsp);

i = CSG::Native.bsp_to_stl(intersect_bsp)
s = CSG::Native.bsp_to_stl(substract_bsp)
u = CSG::Native.bsp_to_stl(union_bsp)

CSG::Native.stl_write_file(u, "union.stl")
CSG::Native.stl_write_file(s, "substract.stl")
CSG::Native.stl_write_file(i, "intersect.stl")
