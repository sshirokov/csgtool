require './lib/csg'

object = CSG::Solid.new(:file => ARGV[0])
object2 = CSG::Solid.new(:file => ARGV[1])

object_bsp = object.tree
object2_bsp = object2.tree

#intersect_bsp = CSG::Native::BSPNode.new( CSG::Native.bsp_intersect(object_bsp, object2_bsp) )
intersect = object.intersect(object2)
subtract = object.subtract(object2)
union = object.union(object2)

i = intersect.to_stl
s = subtract.to_stl
u = union.to_stl

CSG::Native.stl_write_file(u, "union.stl")
CSG::Native.stl_write_file(s, "substract.stl")
CSG::Native.stl_write_file(i, "intersect.stl")
