/**
 * @Description:TyJson的使用方法
 * @Author: hahagiraffe
 * @Date: Created in 下午2:19 2019/8/7
**/

#include "TyJson.h"
#include <stdio.h>
/*JsonValue结构体
struct JsonValue{
    JsonType type;
    union {
        double number;
        struct {
            char* str;
            int len;
        }s;
        struct {
            JsonValue* e;
            size_t size;
        }a;
        struct {
            JsonMember* m;
            size_t  size;
        }o;
    }u;
};
*/
int main(){
    char* str="[ null,13424,{ \"good\" : 123 }]";
    JsonValue value;
    json_parse(&value,str);
    /*\TODO 还需要JSON生成器*/
    char* result;
    int length;
    json_stringify(&value,&result,&length);
    printf("%s\n",result);
    /*注意需要用户手动进行析构*/
    free(result);
    json_free(&value);
    return 0;
}
