#ifndef BASE_STR_H
#define BASE_STR_H

#include <cstddef>
#include <cstdint>

 
int str_copy(char *dst, const char *src, int dst_size);

 
template<int N>
void str_copy(char (&dst)[N], const char *src)
{
	str_copy(dst, src, N);
}

 
void str_append(char *dst, const char *src, int dst_size);

 
template<int N>
void str_append(char (&dst)[N], const char *src)
{
	str_append(dst, src, N);
}

 
void str_truncate(char *dst, int dst_size, const char *src, int truncation_len);

 
int str_length(const char *str);

char str_uppercase(char c);

bool str_isnum(char c);

int str_isallnum(const char *str);

int str_isallnum_hex(const char *str);

 
int str_isspace(char c);

 
const char *str_trim_words(const char *str, int words);

 
bool str_has_cc(const char *str);

 
void str_sanitize_cc(char *str);

 
void str_sanitize(char *str);

 
void str_sanitize_filename(char *str);

 
bool str_valid_filename(const char *str);

 
int str_comp_filenames(const char *a, const char *b);

 
void str_clean_whitespaces(char *str);

 
char *str_skip_to_whitespace(char *str);

 
const char *str_skip_to_whitespace_const(const char *str);

 
char *str_skip_whitespaces(char *str);

 
const char *str_skip_whitespaces_const(const char *str);

 
int str_comp_nocase(const char *a, const char *b);

 
int str_comp_nocase_num(const char *a, const char *b, int num);

 
int str_comp(const char *a, const char *b);

 
int str_comp_num(const char *a, const char *b, int num);

 
const char *str_startswith_nocase(const char *str, const char *prefix);

 
const char *str_startswith(const char *str, const char *prefix);

 
const char *str_endswith_nocase(const char *str, const char *suffix);

 
const char *str_endswith(const char *str, const char *suffix);

 
const char *str_find_nocase(const char *haystack, const char *needle);

 
const char *str_find(const char *haystack, const char *needle);

 
const char *str_next_token(const char *str, const char *delim, char *buffer, int buffer_size);

 
int str_in_list(const char *list, const char *delim, const char *needle);

 
bool str_delimiters_around_offset(const char *haystack, const char *delim, int offset, int *start, int *end);

 
const char *str_rchr(const char *haystack, char needle);

 
int str_countchr(const char *haystack, char needle);

 
void str_hex(char *dst, int dst_size, const void *data, int data_size);

 
void str_hex_cstyle(char *dst, int dst_size, const void *data, int data_size, int bytes_per_line = 12);

 
int str_hex_decode(void *dst, int dst_size, const char *src);

 
void str_base64(char *dst, int dst_size, const void *data, int data_size);

 
int str_base64_decode(void *dst, int dst_size, const char *data);

 
void str_escape(char **dst, const char *src, const char *end);

int str_toint(const char *str);
bool str_toint(const char *str, int *out);
int str_toint_base(const char *str, int base);
unsigned long str_toulong_base(const char *str, int base);
int64_t str_toint64_base(const char *str, int base = 10);
float str_tofloat(const char *str);
bool str_tofloat(const char *str, float *out);

unsigned str_quickhash(const char *str);

 
int str_utf8_encode(char *ptr, int chr);

 
int str_utf8_decode(const char **ptr);

 
void str_utf8_truncate(char *dst, int dst_size, const char *src, int truncation_len);

 
int str_utf8_fix_truncation(char *str);

 
void str_utf8_trim_right(char *param);

 
void str_utf8_tolower(const char *input, char *output, size_t size);

 
int str_utf8_isspace(int code);

 
int str_utf8_isstart(char c);

 
int str_utf8_rewind(const char *str, int cursor);

 
const char *str_utf8_find_nocase(const char *haystack, const char *needle, const char **end = nullptr);

 
int str_utf8_comp_nocase(const char *a, const char *b);

 
int str_utf8_comp_nocase_num(const char *a, const char *b, int num);

 
const char *str_utf8_skip_whitespaces(const char *str);

 
int str_utf8_forward(const char *str, int cursor);

 
int str_utf8_check(const char *str);

 
void str_utf8_copy_num(char *dst, const char *src, int dst_size, int num);

 
void str_utf8_stats(const char *str, size_t max_size, size_t max_count, size_t *size, size_t *count);

 
size_t str_utf8_offset_bytes_to_chars(const char *str, size_t byte_offset);

 
size_t str_utf8_offset_chars_to_bytes(const char *str, size_t char_offset);

#endif
