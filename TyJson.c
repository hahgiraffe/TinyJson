#include "TyJson.h"
#include <assert.h>  /* assert() */
#include <stdio.h>
#define EXPECT(c,ch) do{ assert(*(c->json) == (ch)); c->json++; }while(0)

typedef struct {
    const char* json;
    char* stack;
    size_t size,top;
}JsonContext;

int parse_string(JsonContext* c, JsonValue* v);
int parse_array(JsonContext* c, JsonValue* v);
int parse_value(JsonContext* context,JsonValue* value);
int parse_object(JsonContext* c,JsonValue* v);

/*把空格去掉*/
void parse_whitespace(JsonContext* context){
    const char* json=context->json;
    while(*json==' ' || *json=='\t' || *json=='\n' ||*json=='\r'){
        json++;
    }
    context->json=json;
}
/*解析"null"*/
int parse_null(JsonContext* context,JsonValue* value){
    EXPECT(context,'n');
    if(context->json[0]!='u' || context->json[1]!='l' || context->json[2]!='l'){
        return PARSE_INVALID_VALUE;
    }
    context->json+=3;
    value->type=TYPE_NULL;
    return PARSE_OK;
}

/*解析"true"*/
int parse_true(JsonContext* context,JsonValue* value){
    EXPECT(context,'t');
    if(context->json[0]!='r' || context->json[1]!='u' || context->json[2]!='e'){
        return PARSE_INVALID_VALUE;
    }
    context->json+=3;
    value->type=TYPE_TRUE;
    return PARSE_OK;
}
/*解析"false"*/
int parse_false(JsonContext* context,JsonValue* value){
    EXPECT(context,'f');
    if(context->json[0]!='a' || context->json[1]!='l' || context->json[2]!='s' || context->json[3]!='e'){
        return PARSE_INVALID_VALUE;
    }
    context->json+=4;
    value->type=TYPE_FALSE;
    return PARSE_OK;
}
#define ISDIGIT1TO9(x)  ((x)>='1' && (x)<='9')
#define ISDIGIT(x) ( (x)>='0' && (x)<='9')
/*解析数字*/
int parse_number(JsonContext* context,JsonValue* value){
    int errno;
    const char* json=context->json;
    /*负号*/
    if(*json=='-'){
        json++;
    }
    /*整数*/
    if (*json == '0') json++;
    else {
        if (!ISDIGIT1TO9(*json)) return PARSE_INVALID_VALUE;
        for (json++; ISDIGIT(*json); json++);
    }
    /*小数*/
    if (*json == '.') {
        json++;
        if (!ISDIGIT(*json)) return PARSE_INVALID_VALUE;
        for (json++; ISDIGIT(*json); json++);
    }
    /*指数*/
    if (*json == 'e' || *json == 'E') {
        json++;
        if (*json == '+' || *json == '-') json++;
        if (!ISDIGIT(*json)) return PARSE_INVALID_VALUE;
        for (json++; ISDIGIT(*json); json++);
    }
    value->u.number=strtod(context->json,NULL);
    if ((errno == ERANGE) && (value->u.number == HUGE_VAL) || (value->u.number == -HUGE_VAL))
        return PARSE_NUMBER_TOO_BIG;
    value->type=TYPE_NUMBER;
    context->json=json;
    return  PARSE_OK;
}

int parse_value(JsonContext* context,JsonValue* value){
    switch (*(context->json)){
        case 'n':
            /*printf("into n \n");*/
            return parse_null(context,value);
        case 't':
            /*printf("into t \n");*/
            return parse_true(context,value);
        case 'f':
            /*printf("into f \n");*/
            return parse_false(context,value);
        case '"':
            /*printf("into string \n");*/
            return parse_string(context,value);
        case '[':
            /*printf("into [\n");*/
            return parse_array(context, value);
        case '{':
            /*printf("into {\n")*/
            return parse_object(context,value);
        case '\0':
            return PARSE_EXPECT_VALUE;
        default:
            /*printf("into number \n");*/
            return parse_number(context,value);
    }
}

int json_parse(JsonValue* value,const char* json){
    JsonContext context;
    assert(value != NULL);
    context.json=json;
    context.stack=NULL;
    context.size=context.top=0;
    init(value);
    parse_whitespace(&context);
    int ret;
    /*当解析完空格之后还有字符则返回PARSE_ROOT_NOT_SINGULAR*/
    if((ret=parse_value(&context,value))==PARSE_OK){
        parse_whitespace(&context);
        if(*(context.json)!='\0'){
            value->type=TYPE_NULL;
            ret=PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(context.top==0);
    free(context.stack);
    return ret;
}

JsonType get_type(const JsonValue* value){
    assert(value!=NULL);
    return value->type;
}

double get_number(const JsonValue* value){
    assert(value!=NULL && value->type==TYPE_NUMBER);/*只有类型为TYPE_NUMBER的时候才返回数值*/
    return value->u.number;
}

void set_number(JsonValue* value,double num){
    if(value->type=TYPE_NUMBER)
        value->u.number=num;
}

void set_string(JsonValue* value,const char* s,size_t len){
    json_free(value);
    value->u.s.str=(char*)malloc(len+1);
    memcpy(value->u.s.str,s,len);
    value->type=TYPE_STRING;
    value->u.s.str[len]='\0';
    value->u.s.len=len;
}

/*释放字符串和数组空间*/
void json_free(JsonValue* value){
    size_t i;
    assert(value!=NULL);
    if(value->type==TYPE_STRING){
        free(value->u.s.str);
    }
    if(value->type==TYPE_ARRAY){
        for(i=0;i<value->u.a.size;i++){
            json_free(&value->u.a.e[i]);
        }
        free(value->u.a.e);
    }
    if(value->type==TYPE_OBJECT){
        for(i=0;i<value->u.o.size;i++){
            free(value->u.o.m[i].key);
            json_free(&value->u.o.m[i].value);
        }
        free(value->u.o.m);

    }
    value->type=TYPE_NULL;
}

const char* get_string(JsonValue* value){
    if(value->type==TYPE_STRING)
        return value->u.s.str;
}

size_t get_string_len(JsonValue* value){
    if(value->type==TYPE_STRING)
        return value->u.s.len;
}

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

/*把size的字节push到动态栈中*/
void* context_push(JsonContext* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1;  /* c->size * 1.5 */
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

/*动态栈中的pop*/
void* context_pop(JsonContext* c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

#define PUTC(c, ch) do { *(char*)context_push(c, sizeof(char)) = (ch); } while(0)
#define STRING_ERROR(x) do{  c->top = head; return x;}while(0)

/*把p指的四位数变成16进制*/
char* parese_hex(const char* p,unsigned int* u){
    *u=0;
    int i;
    for(i=0;i<4;i++){
        char c=*p++;
        *u=*u << 4;
        if(c>='0' && c<='9'){
            *u|=c-'0';
        }
        else if(c>='a' && c<='f'){
            *u|=c-'a';
        }
        else if(c>='A' && c<='F'){
            *u|=c-'A';
        }
        else{
            return NULL;
        }
    }
    return p;
}
/*UTF-8编码解析*/
void encode_utf8(JsonContext* c, unsigned int u){
    if(u>= 0x0000 && u <= 0x007F){
        PUTC(c, u & 0xFF);
    }
    else if (u>= 0x0080 && u<=0x07FF){
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | ( u & 0x3F));
    }
    else if (u >= 0x0800 && u <= 0xFFFF) {
        PUTC(c,0xE0 | ((u >> 12) & 0xFF)); /* 0xE0 = 11100000 */
        PUTC(c,0x80 | ((u >>  6) & 0x3F)); /* 0x80 = 10000000 */
        PUTC(c,0x80 | ( u & 0x3F)); /* 0x3F = 00111111 */
    }
    else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >>  6) & 0x3F));
        PUTC(c, 0x80 | ( u & 0x3F));
    }
}
/*
int parse_string(JsonContext* c, JsonValue* v) {
    size_t head = c->top, len;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        unsigned int u;
        switch (ch) {
            case '\"':
                len = c->top - head;
                set_string(v, (const char*)context_pop(c, len), len);
                c->json = p;
                return PARSE_OK;
            case '\0':
                STRING_ERROR(PARSE_MISS_QUOTATION_MARK);
            case '\\':
                switch(*p++){
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u':
                        if(!(p=parese_hex(p,&u))){
                            STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                        }
                        unsigned int u2;
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\')
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = parese_hex(p, &u2)))
                                STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            default:
                PUTC(c, ch);
        }
    }
}
*/

int parse_string_raw(JsonContext* c, char** s,size_t* len){
    size_t head = c->top;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        unsigned int u;
        switch (ch) {
            case '\"':
                *len = c->top - head;
                *s=context_pop(c,*len);
                c->json = p;
                return PARSE_OK;
            case '\0':
                STRING_ERROR(PARSE_MISS_QUOTATION_MARK);
            case '\\':
                switch(*p++){
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u':
                        if(!(p=parese_hex(p,&u))){
                            STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                        }
                        unsigned int u2;
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\')
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = parese_hex(p, &u2)))
                                STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            default:
                PUTC(c, ch);
        }
    }
}

/*解析字符串*/
int parse_string(JsonContext* c, JsonValue* v) {
    int ret;
    char* s;
    size_t len;
    if((ret=parse_string_raw(c,&s,&len)) == PARSE_OK){
        set_string(v,s,len);
    }
    return ret;
}

size_t get_array_size(JsonValue* value){
    assert(value!=NULL && value->type==TYPE_ARRAY);
    return value->u.a.size;
}

JsonValue* get_array_value(JsonValue* value,size_t index){
    assert(value!=NULL && value->type==TYPE_ARRAY);
    assert(index < value->u.a.size);
    return &(value->u.a.e[index]) ;
}

/*解析数组*/
int parse_array(JsonContext* c, JsonValue* v){
    /*printf("-----\n");*/
    size_t size = 0;
    int ret;
    EXPECT(c, '[');
    parse_whitespace(c);
    if (*c->json == ']') {
        c->json++;
        v->type = TYPE_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return PARSE_OK;
    }
    for (;;) {
        JsonValue e;
        init(&e);
        if ((ret = parse_value(c, &e)) != PARSE_OK){
            break;
        }
        memcpy(context_push(c, sizeof(JsonValue)), &e, sizeof(JsonValue));
        size++;
        parse_whitespace(c);
        if (*c->json == ','){
            c->json++;
            parse_whitespace(c);
        }
        else if (*c->json == ']') {
            /*printf("~~~~~~\n");*/
            c->json++;
            v->type = TYPE_ARRAY;
            v->u.a.size = size;
            size *= sizeof(JsonValue);
            memcpy(v->u.a.e = (JsonValue*)malloc(size), context_pop(c, size), size);
            return PARSE_OK;
        }
        else{
            ret=PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    int i;
    for (i = 0; i < size; i++)
        json_free((JsonValue*)context_pop(c, sizeof(JsonValue)));
    return ret;
}

size_t get_object_size(const JsonValue* value){
    assert(value!=NULL && value->type==TYPE_OBJECT);
    return value->u.o.size;
}
const char* get_object_key(const JsonValue* value,size_t index){
    assert(value!=NULL && value->type==TYPE_OBJECT);
    assert(index < value->u.o.size);
    return value->u.o.m[index].key;
}
size_t get_object_keylen(const JsonValue* value,size_t index){
    assert(value!=NULL && value->type==TYPE_OBJECT);
    assert(index < value->u.o.size);
    return value->u.o.m[index].keylen;
}
JsonValue* get_object_value(const JsonValue* value,size_t index){
    assert(value!=NULL && value->type==TYPE_OBJECT);
    assert(index < value->u.o.size);
    return &value->u.o.m[index].value;
}


/*解析json对象*/
int parse_object(JsonContext* c,JsonValue* v){
    size_t size;
    JsonMember m;
    int ret;
    EXPECT(c, '{');
    parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = TYPE_OBJECT;
        v->u.o.m = 0;
        v->u.o.size = 0;
        return PARSE_OK;
    }
    m.key = NULL;
    size = 0;
    for (;;) {
        init(&m.value);
        char* str;
        /* \todo parse key to m.k, m.klen */
        if(*c->json!='"'){
            ret=PARSE_MISS_KEY;
            break;
        }
        if((ret=parse_string_raw(c,&str,&m.keylen))!=PARSE_OK){
            break;
        }
        memcpy(m.key=(char*)malloc(m.keylen+1),str,m.keylen);
        m.key[m.keylen]='\0';
        /* \todo parse ws colon ws */
        parse_whitespace(c);
        if (*c->json != ':') {
            ret = PARSE_MISS_COLON;
            break;
        }
        c->json++;
        parse_whitespace(c);
        /* parse value */
        if ((ret = parse_value(c, &m.value)) != PARSE_OK)
            break;
        memcpy(context_push(c, sizeof(JsonMember)), &m, sizeof(JsonMember));
        size++;
        m.key = NULL; /* ownership is transferred to member on stack */
        /* \todo parse ws [comma | right-curly-brace] ws */
        parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            parse_whitespace(c);
        }
        else if (*c->json == '}') {
            size_t s = sizeof(JsonMember) * size;
            c->json++;
            v->type = TYPE_OBJECT;
            v->u.o.size = size;
            memcpy(v->u.o.m = (JsonMember*)malloc(s), context_pop(c, s), s);
            return PARSE_OK;
        }
        else {
            ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    /* \todo Pop and free members on the stack */
    free(m.key);
    int i;
    for (i = 0; i < size; i++) {
        JsonMember* m = (JsonMember*)context_pop(c, sizeof(JsonMember));
        free(m->key);
        json_free(&m->value);
    }
    v->type = TYPE_NULL;
    return ret;
}

#ifndef LEPT_PARSE_STRINGIFY_INIT_SIZE
#define LEPT_PARSE_STRINGIFY_INIT_SIZE 256
#endif

#define PUTS(c, s, len)     memcpy(context_push(c, len), s, len)

void stringify_string(JsonContext* c,char* str,size_t size){
    size_t i;
    assert(str != NULL);
    PUTC(c, '"');
    for (i = 0; i < size; i++) {
        unsigned char ch = (unsigned char)str[i];
        switch (ch) {
            case '\"': PUTS(c, "\\\"", 2); break;
            case '\\': PUTS(c, "\\\\", 2); break;
            case '\b': PUTS(c, "\\b",  2); break;
            case '\f': PUTS(c, "\\f",  2); break;
            case '\n': PUTS(c, "\\n",  2); break;
            case '\r': PUTS(c, "\\r",  2); break;
            case '\t': PUTS(c, "\\t",  2); break;
            default:
                if (ch < 0x20) {
                    char buffer[7];
                    sprintf(buffer, "\\u%04X", ch);
                    PUTS(c, buffer, 6);
                }
                else
                    PUTC(c, str[i]);
        }
    }
    PUTC(c, '"');
}

int stringify_value(JsonContext* c,JsonValue* v){
    size_t i;
    int ret;
    switch (v->type) {
        case TYPE_NULL:   PUTS(c, "null",  4); break;
        case TYPE_FALSE:  PUTS(c, "false", 5); break;
        case TYPE_TRUE:   PUTS(c, "true",  4); break;
        case TYPE_NUMBER:
            {
                char buffer[32];
                int length = sprintf(buffer, "%.17g", v->u.number);
                PUTS(c, buffer, length);
            }
            break;
        case TYPE_STRING: stringify_string(c, v->u.s.str, v->u.s.len); break;
        case TYPE_ARRAY:
            PUTC(c, '[');
            for (i = 0; i < v->u.a.size; i++) {
                if (i > 0)
                    PUTC(c, ',');
                stringify_value(c, &v->u.a.e[i]);
            }
            PUTC(c, ']');
            break;
        case TYPE_OBJECT:
            PUTC(c, '{');
            for (i = 0; i < v->u.o.size; i++) {
                if (i > 0)
                    PUTC(c, ',');
                stringify_string(c, v->u.o.m[i].key, v->u.o.m[i].keylen);
                PUTC(c, ':');
                stringify_value(c, &v->u.o.m[i].value);
            }
            PUTC(c, '}');
            break;
    }
    return STRINGIFY_OK;
}

int json_stringify(const JsonValue* v,char** json,size_t* len){
    JsonContext c;
    int ret;
    assert(v != NULL);
    assert(json != NULL);
    c.stack = (char*)malloc(c.size = LEPT_PARSE_STRINGIFY_INIT_SIZE);
    c.top = 0;
    if ((ret = stringify_value(&c, v)) != STRINGIFY_OK) {
        free(c.stack);
        *json = NULL;
        return ret;
    }
    if (len)
        *len = c.top;
    PUTC(&c, '\0');
    *json = c.stack;
    return STRINGIFY_OK;
}

