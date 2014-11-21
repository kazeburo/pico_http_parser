require File.expand_path(File.dirname(__FILE__) + '/bench_helper')

require 'pico_http_parser'

request_body = <<REQ
GET /blakjsdfkas HTTP/1.1\r
Host: blooperblorp\r
Cookie: blah=woop\r
\r
REQ
loop = 300000

#File.read(File.expand_path(File.dirname(__FILE__) + '/sample_request.http'))

Benchmark.ips do |x|
  x.time = 5
  x.warmup = 2

  x.report("PicoHTTPParser") {
    env = {}
    PicoHTTPParser.parse_http_request(request_body,env)
  }

  begin
    require 'unicorn'
    include Unicorn
    x.report("Unicorn's HttpParser") {
      parser = HttpParser.new
      parser.buf << request_body
      parser.parse
    }
  rescue LoadError
    puts("Can't benchmark unicorn as it couldn't be loaded.")
  end

  x.compare!
end
