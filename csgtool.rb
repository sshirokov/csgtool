require 'csg'

puts "Reading input", "--"
object = CSG::Solid.new(:file => ARGV[0])
object2 = CSG::Solid.new(:file => ARGV[1])

puts "Object: #{object.inspect}"
puts "Object2: #{object2.inspect}"

puts "Performing operations", "--"
intersect = object.intersect(object2)
subtract = object.subtract(object2)
union = object.union(object2)

puts "Intersect: #{intersect.inspect}"
puts "Union: #{union.inspect}"
puts "Subtract: #{subtract.inspect}"

puts "Writing output", "--"

intersect.write "intersect.stl"
subtract.write "subtract.stl"
union.write "union.stl"
