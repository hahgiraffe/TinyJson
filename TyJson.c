#include "TyJson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#define EXPECT(c,ch) do{ assert(*(c->json) == (ch)); c->json++; }while(0)

typedef struct {
    const char* json;
    char* stack;
    size_t size,top;
}JsonContext;

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
            return parse_null(context,value);
        case 't':
            return parse_true(context,value);
        case 'f':
            return parse_false(context,value);
        case '\0':
            return PARSE_EXPECT_VALUE;
        default:
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
    int ret=0;
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

void json_free(JsonValue* value){
    assert(value!=NULL);
    if(value->type==TYPE_STRING){
        free(value->u.s.str);
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
