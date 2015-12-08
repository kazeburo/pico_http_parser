require "bundler/gem_tasks"
require "rake/extensiontask"

Rake::ExtensionTask.new "pico_http_parser" do |ext|
  ext.lib_dir = "lib/pico_http_parser"
end

require 'rspec/core/rake_task'
RSpec::Core::RakeTask.new(:spec) do |spec|
  spec.pattern = 'spec/*_spec.rb'
  # spec.rspec_opts = ['-cfs']
end

task :test do
  Rake::Task["clobber"].invoke
  Rake::Task["compile"].invoke
  Rake::Task["spec"].invoke
end

task :default => :test

