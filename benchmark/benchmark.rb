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

Benchmark.bmbm(20) do |bm|
  bm.report("PicoHTTPParser") do
    0.upto(loop) do
      env = {}
      PicoHTTPParser.parse_http_request(request_body,env)
    end
  end
  begin
    require 'unicorn'
    include Unicorn
    bm.report("HttpParser") do
      0.upto(loop) do
        parser = HttpParser.new
        parser.buf << request_body
        parser.parse
      end
    end
  rescue LoadError
    puts("Can't benchmark unicorn as it couldn't be loaded.")
  end
end
