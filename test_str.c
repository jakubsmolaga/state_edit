#define TEST_STR
#ifdef TEST_STR
#include <string.h>
#include <assert.h>
#include "str.h"

int
main(void)
{
        String str = str_new();
        str = str_push(str, 'H');
        str = str_push(str, 'e');
        str = str_push(str, 'l');
        str = str_push(str, 'l');
        str = str_push(str, 'o');
        str = str_push(str, ' ');
        str = str_push(str, 'W');
        str = str_push(str, 'o');
        str = str_push(str, 'r');
        str = str_push(str, 'l');
        str = str_push(str, 'd');
        str = str_push(str, '!');
        assert(strcmp(str, "Hello World!") == 0);
        str = str_insert(str, 5, ',');
        assert(strcmp(str, "Hello, World!") == 0);
        str = str_insert(str, 0, 'H');
        assert(strcmp(str, "HHello, World!") == 0);
        str = str_cut(str, 6, 13);
        assert(strcmp(str, "HHello!") == 0);
	str = str_cut(str, 0, 1);
	assert(strcmp(str, "Hello!") == 0);
	str = str_insert_str(str, 5, " World");
	assert(strcmp(str, "Hello World!") == 0);
        str_free(str);
        return 0;
}
#endif
