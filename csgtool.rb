require 'csg'

object = CSG::Solid.new(:file => ARGV[0])
object2 = CSG::Solid.new(:file => ARGV[1])

intersect = object.intersect(object2)
subtract = object.subtract(object2)
union = object.union(object2)

intersect.to_stl.write_file "intersect.stl"
subtract.to_stl.write_file "subtract.stl"
union.to_stl.write_file "union.stl"
