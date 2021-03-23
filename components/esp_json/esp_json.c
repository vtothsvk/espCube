#include "esp_json.h"

cJSON *Create_array_of_anything(cJSON **objects,int array_num)
{
	cJSON *prev = 0;
	cJSON *root;
	root = cJSON_CreateArray();
	for (int i=0;i<array_num;i++) {
		if (!i)	{
			root->child=objects[i];
		} else {
			prev->next=objects[i];
			objects[i]->prev=prev;
		}
		prev=objects[i];
	}
	return root;
}

char *JSON_Types(int type) {
	if (type == cJSON_Invalid) return ("cJSON_Invalid");
	if (type == cJSON_False) return ("cJSON_False");
	if (type == cJSON_True) return ("cJSON_True");
	if (type == cJSON_NULL) return ("cJSON_NULL");
	if (type == cJSON_Number) return ("cJSON_Number");
	if (type == cJSON_String) return ("cJSON_String");
	if (type == cJSON_Array) return ("cJSON_Array");
	if (type == cJSON_Object) return ("cJSON_Object");
	if (type == cJSON_Raw) return ("cJSON_Raw");
	return NULL;
}