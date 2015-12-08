require 'spec_helper'

describe PicoHTTPParser do
  it 'has a version number' do
    expect(PicoHTTPParser::VERSION).not_to be nil
  end

  it 'result of GET /' do
    env = {}
    PicoHTTPParser.parse_http_request("GET /abc?x=%79 HTTP/1.0\r\n\r\n",env)
    expect(env).to match({
      'PATH_INFO'       => '/abc',
      'QUERY_STRING'    => 'x=%79',
      'REQUEST_METHOD'  => "GET",
      'REQUEST_URI'     => '/abc?x=%79',
      'SCRIPT_NAME'     => '',
      'SERVER_PROTOCOL' => 'HTTP/1.0',
    })
  end

  it 'result of POST with headers' do
    env = {}
    req = <<"EOT";
POST /hoge HTTP/1.1\r
Content-Type: text/plain\r
Content-Length: 15\r
Host: example.com\r
User-Agent: hoge\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(req.length)
    expect(env).to match({
      'CONTENT_LENGTH'  => "15",
      'CONTENT_TYPE'    => 'text/plain',
      'HTTP_HOST'       => 'example.com',
      'HTTP_USER_AGENT' => 'hoge',
      'PATH_INFO'       => '/hoge',
      'REQUEST_METHOD'  => "POST",
      'REQUEST_URI'     => '/hoge',
      'QUERY_STRING'    => '',
      'SCRIPT_NAME'     => '',
      'SERVER_PROTOCOL' => 'HTTP/1.1',
    })
  end

  it 'multiline header' do
    env = {}
    req = <<"EOT";
GET / HTTP/1.0\r
Foo: \r
Foo: \r
  abc\r
 de\r
Foo: fgh\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(req.length)
    expect(env).to match({
      'HTTP_FOO'        => ',   abc de, fgh',
      'PATH_INFO'       => '/',
      'QUERY_STRING'    => '',
      'REQUEST_METHOD'  => 'GET',
      'REQUEST_URI'     => '/',
      'SCRIPT_NAME'     => '',
      'SERVER_PROTOCOL' => 'HTTP/1.0',
    })
  end


  it 'url-encoded' do
    env = {}
    req = <<"EOT";
GET /a%20b HTTP/1.0\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(req.length)
    expect(env).to match({
      'PATH_INFO'      => '/a b',
      'REQUEST_METHOD' => 'GET',
      'REQUEST_URI'    => '/a%20b',
      'QUERY_STRING'   => '',
      'SCRIPT_NAME'     => '',
      'SERVER_PROTOCOL' => 'HTTP/1.0',
    })
  end

  it 'invalid char in url-encoded path' do
    env = {}
    req = <<"EOT";
GET /a%2zb HTTP/1.0\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(-1)
    expect(env).to match({})
  end

  it 'particaly url-encoded path' do
    env = {}
    req = <<"EOT";
GET /a%2 HTTP/1.0\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(-1)
    expect(env).to match({})
  end

  it 'uri fragment' do
    env = {}
    req = <<"EOT";
GET /a/b#c HTTP/1.0\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(req.length)
    expect(env).to match({
    'SCRIPT_NAME' => '',
    'PATH_INFO'   => '/a/b',
    'REQUEST_METHOD' => 'GET',
    'REQUEST_URI'    => '/a/b#c',
    'QUERY_STRING'   => '',
    'SERVER_PROTOCOL' => 'HTTP/1.0',
    })
  end

  it 'uri fragment %23 -> #' do
    env = {}
    req = <<"EOT";
GET /a/b%23c HTTP/1.0\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(req.length)
    expect(env).to match({
    'SCRIPT_NAME' => '',
    'PATH_INFO'   => '/a/b#c',
    'REQUEST_METHOD' => 'GET',
    'REQUEST_URI'    => '/a/b%23c',
    'QUERY_STRING'   => '',
    'SERVER_PROTOCOL' => 'HTTP/1.0',
    })
  end


  it 'URI fragment after query string' do
    env = {}
    req = <<"EOT";
GET /a/b?c=d#e HTTP/1.0\r
\r
EOT
    ret = PicoHTTPParser.parse_http_request(req,env)
    expect(ret).to eq(req.length)
    expect(env).to match({
    'PATH_INFO'   => '/a/b',
    'REQUEST_METHOD' => 'GET',
    'REQUEST_URI'    => '/a/b?c=d#e',
    'QUERY_STRING'   => 'c=d',
    'SCRIPT_NAME'     => '',
    'SERVER_PROTOCOL' => 'HTTP/1.0',
    })
  end

end
