#include <ruby.h>
#include <ctype.h>
#include "picohttpparser/picohttpparser.c"

#define MAX_HEADER_NAME_LEN 1024
#define MAX_HEADERS         128
#define TOU(ch) (('a' <= ch && ch <= 'z') ? ch - ('a' - 'A') : ch)

VALUE cPicoHTTPParser;

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

  buf_str = StringValuePtr(buf);
  buf_len = strlen(buf_str);
  num_headers = MAX_HEADERS;
  ret = phr_parse_request(buf_str, buf_len, &method, &method_len, &path,
                          &path_len, &minor_version, headers, &num_headers, 0);
  if (ret < 0)
    goto done;

  rb_hash_aset(envref, rb_str_new2("REQUEST_METHOD"), rb_str_new(method,method_len));
  rb_hash_aset(envref, rb_str_new2("REQUEST_URI"), rb_str_new(path, path_len));
  rb_hash_aset(envref, rb_str_new2("SCRIPT_NAME"), rb_str_new2(""));
  i = sprintf(tmp,"HTTP/1.%d",minor_version);
  rb_hash_aset(envref, rb_str_new2("SERVER_PROTOCOL"), rb_str_new(tmp, i));

  /* PATH_INFO QUERY_STRING */
  path_len = find_ch(path, path_len, '#'); /* strip off all text after # after storing request_uri */
  question_at = find_ch(path, path_len, '?');
  if ( store_path_info(envref, path, question_at) < 0 ) {
    rb_hash_clear(envref);
    ret = -1;
    goto done;
  }
  if (question_at != path_len) ++question_at;
  rb_hash_aset(envref, rb_str_new2("QUERY_STRING"), rb_str_new(path + question_at, path_len - question_at));

  last_value = Qnil;
  for (i = 0; i < num_headers; ++i) {
    if (headers[i].name != NULL) {
      const char* name;
      size_t name_len;
      VALUE slot;
      if (header_is(headers + i, "CONTENT-TYPE", sizeof("CONTENT-TYPE") - 1)) {
        name = "CONTENT_TYPE";
        name_len = sizeof("CONTENT_TYPE") - 1;
      } else if (header_is(headers + i, "CONTENT-LENGTH", sizeof("CONTENT-LENGTH") - 1)) {
        name = "CONTENT_LENGTH";
        name_len = sizeof("CONTENT_LENGTH") - 1;
      } else {
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
        }
      }
      slot = rb_hash_aref(envref, rb_str_new(name, name_len));
      if ( slot != Qnil ) {
        rb_str_cat2(slot, ", ");
        rb_str_cat(slot, headers[i].value, headers[i].value_len);
      } else {
        slot = rb_str_new(headers[i].value, headers[i].value_len);
        rb_hash_aset(envref, rb_str_new(name, name_len), slot);
        last_value = slot;
      }
    } else {
      /* continuing lines of a mulitiline header */
      rb_str_cat(last_value, headers[i].value, headers[i].value_len);
    }
  }

 done:
  return rb_int_new(ret);
}

void Init_pico_http_parser()
{

  cPicoHTTPParser = rb_const_get(rb_cObject, rb_intern("PicoHTTPParser"));
  rb_define_module_function(cPicoHTTPParser, "parse_http_request", phr_parse_http_request, 2);
}
