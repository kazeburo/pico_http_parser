require File.expand_path(File.dirname(__FILE__) + '/bench_helper')

require 'pico_http_parser'

request_bodys = [
  "GET /foo/bar/baz.html?key=value HTTP/1.0\r\nHost: blooperblorp\r\n\r\n",
  "GET /foo/bar/baz.html?key=value HTTP/1.0\r\nHost: blooperblorp\r\nUser-Agent: Mozilla/5.0\r\n\r\n",
  "GET /foo/bar/baz.html?key=value HTTP/1.0\r\nHost: blooperblorp\r\nCookie: foobar\r\nX-Forwarded-For: 127.0.0.1\r\nUser-Agent: Mozilla/5.0\r\n\r\n",
  "GET /foo/bar/baz.html?key=value HTTP/1.0\r\nHost: blooperblorp\r\nCookie: foobar\r\nX-Forwarded-For: 127.0.0.1\r\nUser-Agent: Mozilla/5.0\r\nAccept: X-5\r\nConnection: XXXXXX-6\r\nReferer: XXXXXXXX-7\r\nAccept-Encoding: XXXXXXX8\r\nCache-Control: XXXXXXXX9\r\nIf-Modified-Since: XXXXXXXXXXXXXXX10\r\n\r\n",
  "GET /foo/bar/baz.html?key=value HTTP/1.0\r\n\r\n"
]


request_bodys.each do |request_body|
  puts("benchmark #{request_body}")

  Benchmark.ips do |x|
    x.time = 5
    x.warmup = 1

    x.report("PicoHTTPParser") {
      env = {}
      PicoHTTPParser.parse_http_request(request_body,env)
    }

    begin
      require 'unicorn'
      x.report("Unicorn's HttpParser") {
        parser = Unicorn::HttpParser.new
        parser.buf << request_body
        env = parser.parse
      }
    rescue LoadError
      puts("Can't benchmark unicorn as it couldn't be loaded.")
    end

    begin
      require 'puma/puma_http11'
      x.report("Puma's HttpParser") {
        parser = Puma::HttpParser.new
        env = {}
        nread = parser.execute(env, request_body, 0)
      }
    rescue LoadError
      puts("Can't benchmark puma as it couldn't be loaded.")
    end

    x.compare!
  end
end

