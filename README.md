# PicoHTTPParser

Fast HTTP Parser using picohttpparser.

This module parse HTTP request and inserts variables into Hash. For the name of the variables inserted, please refer to the Rack specification

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'pico_http_parser'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install pico_http_parser

## Usage

```
require 'pico_http_parser'

request_body = <<REQ
GET /blakjsdfkas HTTP/1.1\r
Host: blooperblorp\r
Cookie: blah=woop\r
\r
REQ
env = {}
ret = PicoHTTPParser.parse_http_request(request_body,env)
```

The return values are:

#### >=0

length of the request (request line and the request headers), in bytes

#### -1

given request is corrupt

#### -2

given request is incomplete

## Benchmark

On my Macbook Air

```
$ ruby benchmark.rb
Calculating -------------------------------------
      PicoHTTPParser    13.655k i/100ms
Unicorn's HttpParser    11.554k i/100ms
-------------------------------------------------
      PicoHTTPParser    155.331k (¡Þ 3.7%) i/s -    778.335k
Unicorn's HttpParser    127.455k (¡Þ 3.5%) i/s -    647.024k

Comparison:
      PicoHTTPParser:   155331.2 i/s
Unicorn's HttpParser:   127455.1 i/s - 1.22x slower
```

## SEE ALSO

PicoHTTPParser is created based on Perl's HTTP::Parser::XS. 

https://metacpan.org/pod/HTTP::Parser::XS

This module uses picohttpparser

https://github.com/h2o/picohttpparser

## Contributing

1. Fork it ( https://github.com/kazeburo/pico_http_parser/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
