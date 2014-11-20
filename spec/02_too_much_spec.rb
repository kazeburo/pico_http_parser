require 'spec_helper'

MAX_HEADER_LEN = 1024;
MAX_HEADERS    = 128;
crlf = "\015\012";

describe PicoHTTPParser do

  it 'very long name' do
    env = {}
    name = 'x' * MAX_HEADER_LEN; # OK
    request = "GET / HTTP/1.1" + crlf + name + ": 42" + crlf + crlf;
    ret = PicoHTTPParser.parse_http_request(request,env)
    expect(ret).to be > 0;
    expect(env["REQUEST_METHOD"]).to match("GET")
    expect(env["HTTP_"+name.upcase]).to match("42")
  end


  it 'very long name fail' do
    env = {}
    name = 'x' * (MAX_HEADER_LEN+2); # OK
    request = "GET / HTTP/1.1" + crlf + name + ": 42" + crlf + crlf;
    ret = PicoHTTPParser.parse_http_request(request,env)
    expect(ret).to be == -1;
    expect(env["REQUEST_METHOD"]).to be nil
    expect(env["HTTP_"+name.upcase]).to be nil
  end

  it 'too may headers fail' do
    env = {}
    request = "GET / HTTP/1.1" + crlf +
          (0..MAX_HEADERS).map{|i| "X"+i.to_s+": "+i.to_s}.join(crlf) +
          crlf + crlf
    ret = PicoHTTPParser.parse_http_request(request,env)
    expect(ret).to be == -1;
  end


  it 'too may headers ok' do
    env = {}
    request = "GET / HTTP/1.1" + crlf +
          (1..MAX_HEADERS).map{|i| "X"+i.to_s+": "+i.to_s}.join(crlf) +
          crlf + crlf
    ret = PicoHTTPParser.parse_http_request(request,env)
    expect(ret).to eq(request.length);
  end


end
