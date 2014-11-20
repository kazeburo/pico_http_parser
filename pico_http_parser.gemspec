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
  spec.homepage      = "https://github.com/kazeburo/pico_http_parser"
  spec.license       = "Artistic"
  spec.extensions    = %w[ext/pico_http_parser/extconf.rb]

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.7"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rspec", "~> 3"

  # get an array of submodule dirs by executing 'pwd' inside each submodule
  `git submodule --quiet foreach pwd`.split($\).each do |submodule_path|
    # for each submodule, change working directory to that submodule
    Dir.chdir(submodule_path) do
 
      # issue git ls-files in submodule's directory
      submodule_files = `git ls-files`.split($\)
 
      # prepend the submodule path to create absolute file paths
      submodule_files_fullpaths = submodule_files.map do |filename|
        "#{submodule_path}/#{filename}"
      end
 
      # remove leading path parts to get paths relative to the gem's root dir
      # (this assumes, that the gemspec resides in the gem's root dir)
      submodule_files_paths = submodule_files_fullpaths.map do |filename|
        filename.gsub "#{File.dirname(__FILE__)}/", ""
      end
 
      # add relative paths to gem.files
      spec.files += submodule_files_paths
    end
  end

end
