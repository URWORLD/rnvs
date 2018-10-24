/** Licensed under AGPL 3.0. (C) 2010 David Moreno Montero. http://coralbits.com */
#include <onion/onion.h>
#include <onion/log.h>
#include <onion/block.h>
#include <signal.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SIZE 512

struct HashElem{
	unsigned char *key;
	unsigned char *value;
	struct HashElem *next;
};

struct HashElem* ht[SIZE];

//hashfunction
int hash (unsigned char *str)
{
	unsigned long hash = 5381 * str[0] * str [1];
	int c;
	while (c = *str++)hash = ((hash << 5) + hash) + c;	/* hash * 33 + c */
	if(hash < 0) return (hash * -1) % SIZE;
	return hash % SIZE;
}
//get the element with same key and value and returns it
struct HashElem *get(unsigned char *key)
{
	int index = hash(key);
	printf("GET at index %d\n", index);
	struct HashElem *elem = (struct HashElem*) malloc(sizeof(struct HashElem));
	elem->key = malloc( sizeof( unsigned char ) * 512 );
	elem->value = malloc(sizeof(unsigned char) * 512);
	if(ht[index] == NULL) return NULL;
	elem = ht[index];
	elem->value = ht[index]->value;
	elem->key = ht[index]->key;
	while(elem != NULL)
	{
		if(memcmp(elem->key, key, sizeof(&key)) == 0){
			return elem;
		}
		elem = elem->next;
	}
	return NULL;
}
//set a new element with key and value and returns 0 if success
int set(char *key, char* value){
	printf("SET at index %d\n", hash(key));
	//to iterate
	int index = hash(key);
	struct HashElem *newelem = (struct HashElem*) malloc(sizeof(struct HashElem));
	newelem->next = NULL;
	//fehler
	newelem->key = malloc(sizeof(unsigned char) * 512);
	newelem->value = malloc(sizeof(char) * 512);
	int i;
	for(i = 0; i < strlen((char*)value); i++)
	{
		newelem->value[i] = value[i];
		newelem->value[i+1] = '\0';
	}
	for(i = 0; i < strlen((char*)key); i++)
	{
		newelem->key[i] = key[i];
		newelem->key[i+1] = '\0';
	}
	if(ht[index] == NULL)
	{
		ht[index] = newelem;
		return 0;
	}
	struct HashElem *curr = (struct HashElem*) malloc(sizeof(struct HashElem));
	curr->next = ht[index]->next;
	curr->key = malloc(sizeof(unsigned char) * 512);
	memcpy(curr->key, ht[index]->key, sizeof(&ht[index]->key));
	curr->value = malloc(sizeof(char) * 512);
	memcpy(curr->value, ht[index]->value, sizeof(&value));
	if(memcmp(ht[index]->key, key, sizeof(&key)) == 0)
	{
		memcpy(ht[index]->value, value, sizeof(&value));		
		return 0;
	}
	while(curr->next != NULL)
	{
		if(memcmp(curr->next->key, key, sizeof(&key)) == 0)
		{
			memcpy(curr->next->value, value, sizeof(&value));		
			return 0;		
		}
		curr = curr->next;
	}
	curr->next = newelem;
	return 0;
}
//delete element if it exists, returns elem in both cases, because it's not in ht[] after del()
struct HashElem del(struct HashElem *elem, unsigned char *key)
{
	struct HashElem *curr = (struct HashElem*) malloc(sizeof(struct HashElem));
	curr->key = malloc( sizeof( unsigned char ) * 100 );
	curr->value = malloc(sizeof(char) * 100);
	struct HashElem *prev = (struct HashElem*) malloc(sizeof(struct HashElem));
	prev->key = malloc( sizeof( unsigned char ) * 100 );
	prev->value = malloc(sizeof(char) * 100);

    //get the hash 
    int index = hash(key);
	printf("DEL at index %d\n", index);
	curr = ht[index];
	prev =curr;
	if(curr == NULL) return *elem;
	if(curr->next == NULL){
		ht[index] = NULL;
		return *elem;
	}
    while(curr != NULL) 
	{
		if(memcmp(elem->key, key, sizeof(&key)) == 0) 
		{
			prev->next = curr->next;
			free(curr);
			return *elem;
		}
		
		//go to next cell
		prev = curr;
		curr = curr->next;
		
	}      
	return *elem; 
} 

	/********************
	END OF HASHFUNCTIONS 
	********************/
char *seperate_key(char key[], char valueKey[])
{
	key = strtok(valueKey, "\"");
	strtok(NULL, "\"");
	strtok(NULL, "\"");
	strtok(NULL, "\"");
	strtok(NULL, "\"");
	strtok(NULL, "\"");
	strtok(NULL, "\"");
	key = strtok(NULL, "\"");
	printf("key: %s\n", key);
	return key;
}
	
	
char *seperate_value(char value[], char valueKey[])
{
	strtok(valueKey, "\"");
	strtok(NULL, "\"");
	strtok(NULL, "\"");
	value = strtok(NULL, "\"");
	printf("value: %s\n", value);
	return value;
}

int hello(void *p, onion_request *req, onion_response *res){
	onion_request_flags flag = onion_request_get_flags(req);
	printf("FLAG: %d\n",flag);
	onion_response_write0(res,"Server response:\n");
	if (onion_request_get_query(req, "1")){
		char key[100];
		char value[100];
		char valueKey[500];
		strcpy(key, onion_request_get_query(req, "1"));
		if(flag == 16)
		{
			printf("GET\n");
			//TODO: get elem from HT
			printf("key: %s \n", key);
			struct HashElem *elem = get(key);
			if(elem == NULL) onion_response_set_code(res, 404);
			else 
			{
				onion_response_set_code(res, 200);
				strcpy(value, elem->value);
			}
			onion_response_printf(res,"'data=\n  {\n    \"value\":\"%s\",\n    \"key\":\"%s\"\n  }\n",value, key);
		}
		if(flag == 17)
		{
			printf("POST\n");
			strcpy(valueKey, onion_request_get_put(req, "data"));
			strcpy(key  , seperate_key(key, valueKey));
			strcpy(valueKey, onion_request_get_put(req, "data"));
			strcpy(value, seperate_value(value, valueKey));
			set(key, value);
			onion_response_set_code(res, 201);
			onion_response_printf(res,"'data=\n  {\n    \"value\":\"%s\",\n    \"key\":\"%s\"\n  }\n",value, key);
		  
		}
		if(flag == 21)
		{
			printf("PUT\n");
			strcpy(valueKey, onion_request_get_put(req, "data"));
			strcpy(key  , seperate_key(key, valueKey));
			strcpy(valueKey, onion_request_get_put(req, "data"));
			strcpy(value, seperate_value(value, valueKey));
			struct HashElem *elem = (struct HashElem*) malloc(sizeof(struct HashElem));
			elem->key = malloc( sizeof( unsigned char ) * 512 );
			elem->value = malloc(sizeof(char) * 512);
			//TODO operation
			if(get(key) != NULL)
			{
				//replace element
				del(elem, key);
				set(key, value);
				onion_response_set_code(res, 201);
			}
			else onion_response_set_code(res, 404);
			onion_response_printf(res,"'data=\n  {\n    \"value\":\"%s\",\n    \"key\":\"%s\"\n  }\n",value, key);
		}
		if(flag == 22)
		{
			struct HashElem *elem = (struct HashElem*) malloc(sizeof(struct HashElem));
			elem->key = malloc( sizeof( unsigned char ) * 512 );
			elem->value = malloc(sizeof(char) * 512);
			printf("DEL\n");
			printf("key: %s \n", key);
			strcpy(value, "");
			//TODO: delete elem from ht and get infos from req
			del(elem, key);
			onion_response_set_code(res, 200);
			onion_response_printf(res,"'data=\n  {\n    \"value\":\"%s\",\n    \"key\":\"%s\"\n  }\n",value, key);
		}
	}

	onion_response_set_header(res, "Server", "Onion Server");

	return OCS_PROCESSED;
}

onion *o=NULL;

static void shutdown_server(int _){
	if (o)
		onion_listen_stop(o);
}

int main(int argc, char **argv){
	signal(SIGINT,shutdown_server);
	signal(SIGTERM,shutdown_server);

	o=onion_new(O_POOL);
	onion_set_timeout(o, 5000);
	onion_set_hostname(o,"0.0.0.0");
	onion_set_port(o, "4711");
	onion_url *urls=onion_root_url(o);
	
	onion_url_add(urls, "", hello);
	onion_url_add(urls, "^(.*)$", hello);

	onion_listen(o);
	onion_free(o);
	return 0;
}
