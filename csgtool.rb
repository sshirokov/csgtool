require './lib/csg'

object = CSG::Solid.new(:file => ARGV[0])
object2 = CSG::Solid.new(:file => ARGV[1])

object_bsp = object.tree
object2_bsp = object2.tree

intersect_bsp = CSG::Native::BSPNode.new( CSG::Native.bsp_intersect(object_bsp, object2_bsp) )
substract_bsp = CSG::Native::BSPNode.new( CSG::Native.bsp_subtract(object_bsp, object2_bsp) )
union_bsp = CSG::Native::BSPNode.new( CSG::Native.bsp_union(object_bsp, object2_bsp) )

i = CSG::Native::STLObject.new( CSG::Native.bsp_to_stl(intersect_bsp) )
s = CSG::Native::STLObject.new( CSG::Native.bsp_to_stl(substract_bsp) )
u = CSG::Native::STLObject.new( CSG::Native.bsp_to_stl(union_bsp) )

CSG::Native.stl_write_file(u, "union.stl")
CSG::Native.stl_write_file(s, "substract.stl")
CSG::Native.stl_write_file(i, "intersect.stl")
