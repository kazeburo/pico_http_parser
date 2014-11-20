# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'pico_http_parser/version'

Gem::Specification.new do |spec|
  spec.name          = "pico_http_parser"
  spec.version       = PicoHTTPParser::VERSION
  spec.authors       = ["Masahiro Nagano"]
  spec.email         = ["kazeburo@gmail.com"]
  spec.summary       = %q{Fast HTTP parser using picohttparser}
  spec.description   = %q{Fast HTTP parser using picohttparser}
  spec.homepage      = ""
  spec.license       = "Artistic"
  spec.extensions    = %w[ext/pico_http_parser/extconf.rb]

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.7"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rspec"
end
