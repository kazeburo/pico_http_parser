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
      PicoHTTPParser    15.749k i/100ms
Unicorn's HttpParser    13.564k i/100ms
-------------------------------------------------
      PicoHTTPParser    190.401k (± 3.0%) i/s -    960.689k
Unicorn's HttpParser    153.560k (± 4.9%) i/s -    773.148k

Comparison:
      PicoHTTPParser:   190401.3 i/s
Unicorn's HttpParser:   153560.3 i/s - 1.24x slower
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
