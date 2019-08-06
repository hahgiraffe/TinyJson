/**
 * @Description:
 * @Author: hahagiraffe
 * @Date: Created in 下午1:46 2019/8/5
**/
#ifndef TINYJSON_TYJSON_H
#define TINYJSON_TYJSON_H

#define init(x) do{ x->type=TYPE_NULL; }while(0)
#define set_null(x) json_free(x)

/*声明json的7种类型*/
typedef enum {
    TYPE_NULL,
    TYPE_FALSE,
    TYPE_TRUE,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT
}JsonType;

/*解析返回的状态
 * OK表示返回正常
 * EXPECT_VALUE表示json只含有空白
 * ROOT_NOT_SINGULAR表示一个值之后空白还有其他字符
 * INVALID_VALUE表示不合法返回
 * */
typedef enum {
    PARSE_OK=0,
    PARSE_EXPECT_VALUE,
    PARSE_INVALID_VALUE,
    PARSE_ROOT_NOT_SINGULAR,
    PARSE_NUMBER_TOO_BIG
}Parse_status;

/*Json数据的结构体
 * 因为不会同时为数字或字符串
 * 所以用union，32位机器上能省下4byte
 * */
typedef struct{
    JsonType type;
    union {
        double number;
        struct {
            char* str;
            int len;
        }s;
    }u;
}JsonValue;

/*Json解析函数*/
int json_parse(JsonValue* value,const char* json);

/*返回Json的type*/
JsonType get_type(const JsonValue* value);
/*返回Json的number*/
double get_number(const JsonValue* value);
void set_number(JsonValue* value,double num);

/*设置string分配内存*/
void set_string(JsonValue* value,const char* s,size_t len);
size_t get_string_len(JsonValue* value);
const char* get_string(JsonValue* value);
/*释放内存*/
void json_free(JsonValue* value);

#endif /*TINYJSON_TYJSON_H*/
