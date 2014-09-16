Gem::Specification.new do |s|
  s.name = 'csg'
  s.version = '0.1.4'
  s.summary     = "A fast library for Constructive Solid Geometry"
  s.description = s.summary
  s.authors     = ["Yaroslav Shirokov", "Sean Bryant"]
  s.email       = ['sshirokov@github.com', 'sbryant@github.com']
  s.license     = 'MIT'
  s.files       = ["Makefile"] + Dir["lib/**/*.rb"] + Dir['src/**/*.{c,h}']
  s.homepage    = 'https://github.com/sshirokov/csgtool/'
  s.add_runtime_dependency 'ffi'
  s.extensions = ['ext/Rakefile']
end
