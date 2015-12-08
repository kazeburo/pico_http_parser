#include <ruby.h>
#include <ctype.h>
#include "picohttpparser/picohttpparser.c"

#define MAX_HEADER_NAME_LEN 1024
#define MAX_HEADERS         128
#define TOU(ch) (('a' <= ch && ch <= 'z') ? ch - ('a' - 'A') : ch)

VALUE cPicoHTTPParser;

static VALUE request_method_key;
static VALUE request_uri_key;
static VALUE script_name_key;
static VALUE server_protocol_key;
static VALUE query_string_key;

static VALUE vacant_string_val;

static VALUE http10_val;
static VALUE http11_val;

struct common_header {
  const char * name;
  size_t name_len;
  VALUE key;
};
static int common_headers_num = 0;
static struct common_header common_headers[20];

static
void set_common_header(const char * key, int key_len, const int raw)
{
  char tmp[MAX_HEADER_NAME_LEN + sizeof("HTTP_") - 1];
  const char* name;
  size_t name_len;
  const char * s;
  char* d;
  size_t n;
  VALUE env_key;

  if ( raw == 1) {
    for (s = key, n = key_len, d = tmp;
      n != 0;
      s++, --n, d++) {
      *d = *s == '-' ? '_' : TOU(*s);
      name = tmp;
      name_len = key_len;
    }
  } else {
    strcpy(tmp, "HTTP_");
    for (s = key, n = key_len, d = tmp + 5;
      n != 0;
      s++, --n, d++) {
      *d = *s == '-' ? '_' : TOU(*s);
      name = tmp;
      name_len = key_len + 5;
    }
  }
  env_key = rb_obj_freeze(rb_str_new(name,name_len));
  common_headers[common_headers_num].name = key;
  common_headers[common_headers_num].name_len = key_len;
  common_headers[common_headers_num].key = env_key;
  rb_gc_register_address(&common_headers[common_headers_num].key);
  common_headers_num++;
}


static
size_t find_ch(const char* s, size_t len, char ch)
{
  size_t i;
  for (i = 0; i != len; ++i, ++s)
    if (*s == ch)
      break;
  return i;
}

static
int header_is(const struct phr_header* header, const char* name,
                    size_t len)
{
  const char* x, * y;
  if (header->name_len != len)
    return 0;
  for (x = header->name, y = name; len != 0; --len, ++x, ++y)
    if (TOU(*x) != *y)
      return 0;
  return 1;
}

static
VALUE find_common_header(const struct phr_header* header) {
  int i;
  for ( i = 0; i < common_headers_num; i++ ) {
    if ( header_is(header, common_headers[i].name, common_headers[i].name_len) ) {
      return common_headers[i].key;
    }
  }
  return Qnil;
}

static
int store_path_info(VALUE envref, const char* src, size_t src_len) {
  size_t dlen = 0, i = 0;
  char *d;
  char s2, s3;

  d = (char*)malloc(src_len * 3 + 1);
  for (i = 0; i < src_len; i++ ) {
    if ( src[i] == '%' ) {
      if ( !isxdigit(src[i+1]) || !isxdigit(src[i+2]) ) {
        free(d);
        return -1;
      }
      s2 = src[i+1];
      s3 = src[i+2];
      s2 -= s2 <= '9' ? '0'
          : s2 <= 'F' ? 'A' - 10
          : 'a' - 10;
      s3 -= s3 <= '9' ? '0'
          : s3 <= 'F' ? 'A' - 10
          : 'a' - 10;
       d[dlen++] = s2 * 16 + s3;
       i += 2;
    }
    else {
      d[dlen++] = src[i];
    }
  }
  d[dlen]='0';
  rb_hash_aset(envref, rb_str_new2("PATH_INFO"), rb_str_new(d, dlen));
  free(d);
  return dlen;
}


static
VALUE phr_parse_http_request(VALUE self, VALUE buf, VALUE envref)
{
  const char* buf_str;
  size_t buf_len;
  const char* method;
  size_t method_len;
  const char* path;
  size_t path_len;
  size_t o_path_len;
  int minor_version;
  struct phr_header headers[MAX_HEADERS];
  size_t num_headers, question_at;
  size_t i;
  int ret;
  char tmp[MAX_HEADER_NAME_LEN + sizeof("HTTP_") - 1];
  VALUE last_value;

  buf_str = RSTRING_PTR(buf);
  buf_len = RSTRING_LEN(buf);
  num_headers = MAX_HEADERS;
  ret = phr_parse_request(buf_str, buf_len, &method, &method_len, &path,
                          &path_len, &minor_version, headers, &num_headers, 0);
  if (ret < 0)
    goto done;

  rb_hash_aset(envref, request_method_key, rb_str_new(method,method_len));
  rb_hash_aset(envref, request_uri_key, rb_str_new(path, path_len));
  rb_hash_aset(envref, script_name_key, vacant_string_val);
  rb_hash_aset(envref, server_protocol_key, (minor_version == 1) ? http11_val : http10_val);

  /* PATH_INFO QUERY_STRING */
  path_len = find_ch(path, path_len, '#'); /* strip off all text after # after storing request_uri */
  question_at = find_ch(path, path_len, '?');
  if ( store_path_info(envref, path, question_at) < 0 ) {
    rb_hash_clear(envref);
    ret = -1;
    goto done;
  }
  if (question_at != path_len) ++question_at;
  rb_hash_aset(envref, query_string_key, rb_str_new(path + question_at, path_len - question_at));
 
  last_value = Qnil;
  for (i = 0; i < num_headers; ++i) {
    if (headers[i].name != NULL) {
      const char* name;
      size_t name_len;
      VALUE slot;
      VALUE env_key;
      env_key = find_common_header(headers + i);
      if ( env_key == Qnil ) {
        const char* s;
        char* d;
        size_t n;
        if (sizeof(tmp) - 5 < headers[i].name_len) {
          rb_hash_clear(envref);
          ret = -1;
          goto done;
        }
        strcpy(tmp, "HTTP_");
        for (s = headers[i].name, n = headers[i].name_len, d = tmp + 5;
          n != 0;
          s++, --n, d++) {
            *d = *s == '-' ? '_' : TOU(*s);
            name = tmp;
            name_len = headers[i].name_len + 5;
            env_key = rb_str_new(name, name_len);
        }
      }
      
      slot = rb_hash_aref(envref, env_key);
      if ( slot != Qnil ) {
        rb_str_cat2(slot, ", ");
        rb_str_cat(slot, headers[i].value, headers[i].value_len);
      } else {
        slot = rb_str_new(headers[i].value, headers[i].value_len);
        rb_hash_aset(envref, env_key, slot);
        last_value = slot;
      }
      
    } else {
      /* continuing lines of a mulitiline header */
      if ( last_value != Qnil )
        rb_str_cat(last_value, headers[i].value, headers[i].value_len);
    }
    
  }

 done:
  return rb_int_new(ret);
}

void Init_pico_http_parser()
{
  request_method_key = rb_obj_freeze(rb_str_new2("REQUEST_METHOD"));
  rb_gc_register_address(&request_method_key);
  request_uri_key = rb_obj_freeze(rb_str_new2("REQUEST_URI"));
  rb_gc_register_address(&request_uri_key);
  script_name_key = rb_obj_freeze(rb_str_new2("SCRIPT_NAME"));
  rb_gc_register_address(&script_name_key);
  server_protocol_key = rb_obj_freeze(rb_str_new2("SERVER_PROTOCOL"));
  rb_gc_register_address(&server_protocol_key);
  query_string_key = rb_obj_freeze(rb_str_new2("QUERY_STRING"));
  rb_gc_register_address(&query_string_key);

  set_common_header("ACCEPT",sizeof("ACCEPT") - 1, 0);
  set_common_header("ACCEPT-ENCODING",sizeof("ACCEPT-ENCODING") - 1, 0);
  set_common_header("ACCEPT-LANGUAGE",sizeof("ACCEPT-LANGUAGE") - 1, 0);
  set_common_header("CACHE-CONTROL",sizeof("CACHE-CONTROL") - 1, 0);
  set_common_header("CONNECTION",sizeof("CONNECTION") - 1, 0);
  set_common_header("CONTENT-LENGTH",sizeof("CONTENT-LENGTH") - 1, 1);
  set_common_header("CONTENT-TYPE",sizeof("CONTENT-TYPE") - 1, 1);
  set_common_header("COOKIE",sizeof("COOKIE") - 1, 0);
  set_common_header("HOST",sizeof("HOST") - 1, 0);
  set_common_header("IF-MODIFIED-SINCE",sizeof("IF-MODIFIED-SINCE") - 1, 0);
  set_common_header("REFERER",sizeof("REFERER") - 1, 0);
  set_common_header("USER-AGENT",sizeof("USER-AGENT") - 1, 0);
  set_common_header("X-FORWARDED-FOR",sizeof("X-FORWARDED-FOR") - 1, 0);

  http10_val = rb_obj_freeze(rb_str_new2("HTTP/1.0"));
  rb_gc_register_address(&http10_val);
  http11_val = rb_obj_freeze(rb_str_new2("HTTP/1.1"));
  rb_gc_register_address(&http11_val);
  vacant_string_val = rb_obj_freeze(rb_str_new("",0));
  rb_gc_register_address(&vacant_string_val);

  cPicoHTTPParser = rb_const_get(rb_cObject, rb_intern("PicoHTTPParser"));
  rb_define_module_function(cPicoHTTPParser, "parse_http_request", phr_parse_http_request, 2);
}
