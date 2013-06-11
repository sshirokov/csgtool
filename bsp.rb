require 'ffi'

module CSG
  extend FFI::Library

  ffi_lib 'csg'

  FLOAT3 = FFI::ArrayType.new(FFI::Type::FLOAT, 3)
  typedef FLOAT3, :float3

  class STLFacet < FFI::Struct
    layout :normal, :float3,
           :vertices, FFI::ArrayType.new(CSG::FLOAT3, 3),
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
end



stl_object = CSG.stl_read_file("nyx.stl", true)
CSG.stl_write_file(stl_object, "nyx_test.stl")
