csgtool [![Build Status](https://travis-ci.org/sshirokov/csgtool.png?branch=master)](https://travis-ci.org/sshirokov/csgtool)
=======

**CSGTOOL** is a library, [Ruby Gem](http://rubygems.org/gems/csg) and command line tool for performing
[Constructive Solid Geometry](http://en.wikipedia.org/wiki/Constructive_solid_geometry) operations on
[STL Files](http://bit.ly/wikistl) using [3D BSP Trees](http://en.wikipedia.org/wiki/Binary_space_partitioning).

The library is written in C99 with performance, portability and readability as primary goals. The ruby gem wraps the library
using the [ffi gem](leaning towards, and requ) to provide a friendlier interface to the underlying engine.

### Quickstart

```
$ git clone https://github.com/sshirokov/csgtool.git
$ make test
$ bundle exec ruby ./csgtool.rb tests/fixtures/jaws.stl tests/fixtures/jaws2.stl
```

This should result in three files: `intersect.stl`, `subtract.stl`, and `union.stl` representing
the CSG operations performed on the arguments.

![intersect](http://f.cl.ly/items/3s190I193g0f0z2l0a3v/Image%202013.06.22%2012%3A15%3A20%20PM.png)

![subtract](http://f.cl.ly/items/2g113v3h422R2P463Q2L/Image%202013.06.22%2012%3A15%3A08%20PM.png)

![union](http://f.cl.ly/items/203S11222f0s3D2c120e/Image%202013.06.22%2012%3A15%3A30%20PM.png)