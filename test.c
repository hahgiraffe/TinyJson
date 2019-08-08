/*
 　TDD测试开发模式
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TyJson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
do {\
    test_count++;\
    if (equality)\
        test_pass++;\
    else {\
        fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
        main_ret = 1;\
    }\
} while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect,actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lf")

void test_parse_null() {
    JsonValue v;
    v.type = TYPE_FALSE;
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "null"));
    EXPECT_EQ_INT(TYPE_NULL, get_type(&v));
}

void test_parse_true() {
    JsonValue v;
    v.type = TYPE_FALSE;
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "true"));
    EXPECT_EQ_INT(TYPE_TRUE, get_type(&v));
}

void test_parse_false() {
    JsonValue v;
    v.type = TYPE_TRUE;
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "false"));
    EXPECT_EQ_INT(TYPE_FALSE, get_type(&v));
}

#define TEST_ERROR(error,json) \
    do {\
        JsonValue v;\
        v.type = TYPE_FALSE;\
        EXPECT_EQ_INT(error, json_parse(&v, json));\
        EXPECT_EQ_INT(TYPE_NULL, get_type(&v));\
    } while(0)

void test_parse_expect_value() {
    TEST_ERROR(PARSE_EXPECT_VALUE,"");
    TEST_ERROR(PARSE_EXPECT_VALUE," ");
}

void test_parse_invalid_value() {
    TEST_ERROR(PARSE_INVALID_VALUE,"nul");
    TEST_ERROR(PARSE_INVALID_VALUE,"?");
#if 1
    TEST_ERROR(PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PARSE_INVALID_VALUE, "nan");
#endif
}

void test_parse_root_not_singular() {
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");
#if 1
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}


void test_parse_number_too_big() {
#if 1
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

#define TEST_NUMBER(expect, json) \
    do{ \
        JsonValue v;\
        EXPECT_EQ_INT(PARSE_OK, json_parse(&v, json));\
        EXPECT_EQ_INT(TYPE_NUMBER, get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, get_number(&v));\
    }while(0)

void test_parse_number(){
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

#define EXPECT_EQ_STRING(expect,actual,len) EXPECT_EQ_BASE(sizeof(expect) - 1 == len && memcmp(expect, actual, len) == 0, expect, actual, "%s")
#define TEST_STRING(expect, json)\
    do {\
        JsonValue v;\
        init(&v);\
        EXPECT_EQ_INT(PARSE_OK, json_parse(&v, json));\
        EXPECT_EQ_INT(TYPE_STRING, get_type(&v));\
        EXPECT_EQ_STRING(expect, get_string(&v), get_string_len(&v));\
        json_free(&v);\
    } while(0)

void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
#if 1
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif
}

static void test_access_null() {
    JsonValue v;
    init(&v);
    set_string(&v, "a", 1);
    set_null(&v);
    EXPECT_EQ_INT(TYPE_NULL, get_type(&v));
    json_free(&v);
}
#define EXPECT_TRUE(v)\
    do{\
        EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "true")); \
        EXPECT_EQ_INT(TYPE_TRUE, get_type(&v)); \
    }while(0) \

#define EXPECT_FALSE(v)\
    do{\
        EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "false")); \
        EXPECT_EQ_INT(TYPE_FALSE, get_type(&v)); \
    }while(0) \


static void test_access_boolean() {
    /* Use EXPECT_TRUE() and EXPECT_FALSE() */
    JsonValue v;
    init(&v);
    v.type=TYPE_TRUE;
    EXPECT_TRUE(v);
    v.type=TYPE_FALSE;
    EXPECT_FALSE(v);
    json_free(&v);
}

static void test_access_number() {
    JsonValue v;
    init(&v);
    set_number(&v,5.12);
    EXPECT_EQ_DOUBLE(5.12,get_number(&v));
    set_number(&v,0.0);
    EXPECT_EQ_DOUBLE(0.0,get_number(&v));
    json_free(&v);
}

static void test_access_string() {
    JsonValue v;
    init(&v);
    set_string(&v, "", 0);
    EXPECT_EQ_STRING("", get_string(&v), get_string_len(&v));
    set_string(&v, "Hello", 5);
    EXPECT_EQ_STRING("Hello", get_string(&v), get_string_len(&v));
    json_free(&v);
}


#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

static void test_parse_array() {
    JsonValue v;
    init(&v);
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "[ ]"));
    EXPECT_EQ_INT(TYPE_ARRAY, get_type(&v));
    EXPECT_EQ_SIZE_T(0, get_array_size(&v));

    /*[ null , false , true , 123 , "abc" ]
    [ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]*/

    init(&v);
    printf("begin\n");
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(TYPE_ARRAY, get_type(&v));
    EXPECT_EQ_SIZE_T(5, get_array_size(&v));

    EXPECT_EQ_INT(TYPE_NULL,  get_type(get_array_value(&v, 0)));
    EXPECT_EQ_INT(TYPE_FALSE,  get_type(get_array_value(&v, 1)));
    EXPECT_EQ_INT(TYPE_TRUE,   get_type(get_array_value(&v, 2)));
    EXPECT_EQ_INT(TYPE_NUMBER, get_type(get_array_value(&v, 3)));
    EXPECT_EQ_INT(TYPE_STRING, get_type(get_array_value(&v, 4)));
    EXPECT_EQ_DOUBLE(123.0, get_number(get_array_value(&v, 3)));
    EXPECT_EQ_STRING("abc", get_string(get_array_value(&v, 4)), get_string_len(get_array_value(&v, 4)));

    json_free(&v);

    init(&v);
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(TYPE_ARRAY, get_type(&v));
    EXPECT_EQ_SIZE_T(4, get_array_size(&v));
    int i,j;
    for (i = 0; i < 4; i++) {
        JsonValue* a = get_array_value(&v, i);
        EXPECT_EQ_INT(TYPE_ARRAY, get_type(a));
        EXPECT_EQ_SIZE_T(i, get_array_size(a));
        for (j = 0; j < i; j++) {
            JsonValue* e = get_array_value(a, j);
            EXPECT_EQ_INT(TYPE_NUMBER, get_type(e));
            EXPECT_EQ_DOUBLE((double)j, get_number(e));
        }
    }
    json_free(&v);

}

void test_parse_miss_key() {
    TEST_ERROR(PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{\"a\":1,");
}

void test_parse_miss_colon() {
    TEST_ERROR(PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(PARSE_MISS_COLON, "{\"a\",\"b\"}");
}


void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse_object() {
    JsonValue v;
    size_t i;

    init(&v);
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v, " { } "));
    EXPECT_EQ_INT(TYPE_OBJECT, get_type(&v));
    EXPECT_EQ_SIZE_T(0, get_object_size(&v));
    json_free(&v);

    init(&v);
    EXPECT_EQ_INT(PARSE_OK, json_parse(&v,
                                            " { "
                                            "\"n\" : null , "
                                            "\"f\" : false , "
                                            "\"t\" : true , "
                                            "\"i\" : 123 , "
                                            "\"s\" : \"abc\", "
                                            "\"a\" : [ 1, 2, 3 ],"
                                            "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                            " } "
    ));
    EXPECT_EQ_INT(TYPE_OBJECT, get_type(&v));
    EXPECT_EQ_SIZE_T(7, get_object_size(&v));
    EXPECT_EQ_STRING("n", get_object_key(&v, 0), get_object_keylen(&v, 0));
    EXPECT_EQ_INT(TYPE_NULL,   get_type(get_object_value(&v, 0)));
    EXPECT_EQ_STRING("f", get_object_key(&v, 1), get_object_keylen(&v, 1));
    EXPECT_EQ_INT(TYPE_FALSE,  get_type(get_object_value(&v, 1)));
    EXPECT_EQ_STRING("t", get_object_key(&v, 2), get_object_keylen(&v, 2));
    EXPECT_EQ_INT(TYPE_TRUE,   get_type(get_object_value(&v, 2)));
    EXPECT_EQ_STRING("i", get_object_key(&v, 3), get_object_keylen(&v, 3));
    EXPECT_EQ_INT(TYPE_NUMBER, get_type(get_object_value(&v, 3)));
    EXPECT_EQ_DOUBLE(123.0, get_number(get_object_value(&v, 3)));
    EXPECT_EQ_STRING("s", get_object_key(&v, 4), get_object_keylen(&v, 4));
    EXPECT_EQ_INT(TYPE_STRING, get_type(get_object_value(&v, 4)));
    EXPECT_EQ_STRING("abc", get_string(get_object_value(&v, 4)), get_string_len(get_object_value(&v, 4)));
    EXPECT_EQ_STRING("a", get_object_key(&v, 5), get_object_keylen(&v, 5));
    EXPECT_EQ_INT(TYPE_ARRAY, get_type(get_object_value(&v, 5)));
    EXPECT_EQ_SIZE_T(3, get_array_size(get_object_value(&v, 5)));
    for (i = 0; i < 3; i++) {
        JsonValue* e = get_array_value(get_object_value(&v, 5), i);
        EXPECT_EQ_INT(TYPE_NUMBER, get_type(e));
        EXPECT_EQ_DOUBLE(i + 1.0, get_number(e));
    }
    EXPECT_EQ_STRING("o", get_object_key(&v, 6), get_object_keylen(&v, 6));
    {
        JsonValue* o = get_object_value(&v, 6);
        EXPECT_EQ_INT(TYPE_OBJECT, get_type(o));
        for (i = 0; i < 3; i++) {
            JsonValue* ov = get_object_value(o, i);
            int check;
            if('1' + i == get_object_key(o, i)[0]){
                check=1;

            }else{
                check=0;
            }
            EXPECT_EQ_BASE((check) != 0, "true", "false", "%s");
            EXPECT_EQ_SIZE_T(1, get_object_keylen(o, i));
            EXPECT_EQ_INT(TYPE_NUMBER, get_type(ov));
            EXPECT_EQ_DOUBLE(i + 1.0, get_number(ov));
        }
    }
    json_free(&v);
}

#define TEST_ROUNDTRIP(json)\
    do {\
        JsonValue v;\
        char* json2;\
        size_t length;\
        init(&v);\
        EXPECT_EQ_INT(PARSE_OK, json_parse(&v, json));\
        EXPECT_EQ_INT(STRINGIFY_OK, json_stringify(&v, &json2, &length));\
        EXPECT_EQ_STRING(json, json2, length);\
        json_free(&v);\
        free(json2);\
    } while(0)

void test_stringify() {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    TEST_ROUNDTRIP("123313.213");
    TEST_ROUNDTRIP("[null,true,false,2]");
    return ;
}



void test_parse() {

    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_number_too_big();
    test_parse_string();

    test_parse_array();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();

#if 1
    test_parse_object();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
#endif

    test_stringify();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
