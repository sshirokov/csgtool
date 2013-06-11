root = File.expand_path(File.join(File.dirname(File.expand_path(__FILE__)), ".."))

system("make -C #{root} libcsg")
