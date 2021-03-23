#ifndef esp_json_h
#define esp_json_h

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "cJSON.h"

cJSON *Create_array_of_anything(cJSON **objects,int array_num);
char *JSON_Types(int type);
void JSON_Array(const cJSON * const array);

#ifdef __cplusplus
}
#endif

#endif