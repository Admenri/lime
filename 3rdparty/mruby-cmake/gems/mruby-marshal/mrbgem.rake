MRuby::Gem::Specification.new('mruby-marshal-c') do |spec|
  spec.license = 'Public domain'
  spec.authors = 'Yukihiro Matsumoto, CRuby Contributors, CheYoki (C++ Port)'
  spec.summary = 'C++ implementation of the Marshal module for mruby.'

  spec.cc.flags += ['--std=c++20']
  spec.cxx.flags += ['--std=c++20']
end