#pragma once

/* -------------------------------------------------------------------------- */
/*                                 String API                                 */
/* -------------------------------------------------------------------------- */

typedef char* String;
static String str_new(void);
static String str_push(String str, char c);
static String str_insert(String str, int index, char c);
static String str_insert_str(String str, int index, const char *insert);
static String str_cut(String str, int beg, int end);
static void str_free(String str);

/* -------------------------------------------------------------------------- */
/*                               Implementation                               */
/* -------------------------------------------------------------------------- */

#include <stdlib.h>

typedef struct {
        int len;
        int cap;
} _StrHdr;

static _StrHdr*
_str_hdr(String str)
{
        return (_StrHdr*)(str - sizeof(_StrHdr));
}

static _StrHdr*
_str_ensure(_StrHdr *hdr, int cap)
{
        if (hdr->cap >= cap) {
                return hdr;
        }
        while (hdr->cap < cap) {
                hdr->cap *= 2;
        }
        hdr = (_StrHdr*)realloc(hdr, sizeof(_StrHdr) + hdr->cap);
        return hdr;
}

static String 
str_new(void)
{
        int cap = 16;
        _StrHdr* hdr = (_StrHdr*)malloc(sizeof(_StrHdr) + cap);
        hdr->len = 0;
        hdr->cap = cap;
        String str = (String)(&hdr[1]);
        str[0] = '\0';
        return str;
}

static String
str_make(const char *str)
{
	int len = 0;
	while (str[len] != '\0') {
		++len;
	}
	_StrHdr* hdr = (_StrHdr*)malloc(sizeof(_StrHdr) + len + 1);
	hdr->len = len;
	hdr->cap = len + 1;
	String new_str = (String)(&hdr[1]);
	for (int i = 0; i < len; ++i) {
		new_str[i] = str[i];
	}
	new_str[len] = '\0';
	return new_str;
}

static String
str_push(String str, char c)
{
        _StrHdr* hdr = _str_hdr(str);
        hdr = _str_ensure(hdr, hdr->len + 2);
        str = (String)(&hdr[1]);
        str[hdr->len++] = c;
        str[hdr->len] = '\0';
        return str;
}

static String
str_insert(String str, int index, char c)
{
        _StrHdr* hdr = _str_hdr(str);
        hdr = _str_ensure(hdr, hdr->len + 2);
        str = (String)(&hdr[1]);
        for (int i = hdr->len; i > index; --i) {
                str[i] = str[i - 1];
        }
        str[index] = c;
        hdr->len += 1;
        str[hdr->len] = '\0';
        return str;
}

static String
str_insert_str(String str, int index, const char *insert)
{
	_StrHdr* hdr = _str_hdr(str);
	int insert_len = 0;
	while (insert[insert_len] != '\0') {
		++insert_len;
	}
	hdr = _str_ensure(hdr, hdr->len + insert_len + 1);
	str = (String)(&hdr[1]);
	for (int i = hdr->len; i >= index; --i) {
		str[i + insert_len] = str[i];
	}
	for (int i = 0; i < insert_len; ++i) {
		str[index + i] = insert[i];
	}
	hdr->len += insert_len;
	str[hdr->len] = '\0';
	return str;
}

static String
str_cut(String str, int beg, int end)
{
        _StrHdr* hdr = _str_hdr(str);
        beg = beg < 0 ? 0 : beg;
        end = end > hdr->len ? hdr->len : end;
        for (int i = beg; i + end - beg < hdr->len; ++i) {
                str[i] = str[i + end - beg];
        }
        hdr->len -= (end - beg);
        str[hdr->len] = '\0';
        return str;
}

static void
str_free(String str)
{
        if (str == NULL) {
                return;
        }
        _StrHdr* hdr = _str_hdr(str);
        free(hdr);
}
