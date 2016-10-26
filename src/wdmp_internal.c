/**
 * Copyright 2016 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include "wdmp-c.h"
#include "wdmp_internal.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/


void parse_get_request(cJSON *request, req_struct **reqObj)
{
	cJSON *paramArray = NULL;	
	size_t paramCount, i;	
	
	(*reqObj)->u.getReq = (get_req_t *) malloc(sizeof(get_req_t));
	memset((*reqObj)->u.getReq,0,(sizeof(get_req_t)));

	char *param = NULL;
	(*reqObj)->reqType = GET;
	printf("(*reqObj)->reqType : %d\n",(*reqObj)->reqType);
	paramArray = cJSON_GetObjectItem(request, "names");
	paramCount = cJSON_GetArraySize(paramArray);
	(*reqObj)->u.getReq->paramCnt = paramCount;
	printf("(*reqObj)->u.getReq->paramCnt : %lu\n",(*reqObj)->u.getReq->paramCnt);
	
	for (i = 0; i < paramCount; i++) 
	{
		(*reqObj)->u.getReq->paramNames[i] = cJSON_GetArrayItem(paramArray, i)->valuestring;
		printf("(*reqObj)->u.getReq->paramNames[%lu] : %s\n",i,(*reqObj)->u.getReq->paramNames[i]);
	}

	if(cJSON_GetObjectItem(request, "attributes") != NULL) 
	{
		if(strlen(cJSON_GetObjectItem(request, "attributes")->valuestring) != 0)
		{	
			if(strncmp(cJSON_GetObjectItem(request, "attributes")->valuestring, "notify", 6) == 0)
			{
				(*reqObj)->reqType = GET_ATTRIBUTES;
				printf("(*reqObj)->reqType : %d\n",(*reqObj)->reqType);
				for (i = 0; i < paramCount; i++) 
				{
					param = (cJSON_GetArrayItem(paramArray, i)->valuestring);
					printf("param : %s\n",param);
					(*reqObj)->u.getReq->paramNames[i] = param;
					printf("(*reqObj)->u.getReq->paramNames[%lu] : %s\n",i,(*reqObj)->u.getReq->paramNames[i]);			
				}
			}
		}
		else
		{	
			printf("Empty notify received\n");
			wdmp_free_req_struct(*reqObj );
			(*reqObj) = NULL;
		}
	}
	
}

void parse_set_request(cJSON *request, req_struct **reqObj)
{

	cJSON *reqParamObj = NULL, *attributes = NULL,*paramArray = NULL;	
	size_t paramCount, i;
	int notification;
	char notif[20] = "";
	
	printf("parsing Set Request\n");
	
	(*reqObj)->reqType = SET_ATTRIBUTES;
	printf("(*reqObj)->reqType : %d\n",(*reqObj)->reqType);
	paramArray = cJSON_GetObjectItem(request, "parameters");
	paramCount = cJSON_GetArraySize(paramArray);

	(*reqObj)->u.setReq = (set_req_t *) malloc(sizeof(set_req_t));
	memset((*reqObj)->u.setReq,0,(sizeof(set_req_t)));

	(*reqObj)->u.setReq->paramCnt = paramCount;
	printf("(*reqObj)->u.setReq->paramCnt : %lu\n",(*reqObj)->u.setReq->paramCnt);
	(*reqObj)->u.setReq->param = (param_t *) malloc(sizeof(param_t) * paramCount);
	memset((*reqObj)->u.setReq->param,0,(sizeof(param_t) * paramCount));

	for (i = 0; i < paramCount; i++) 
	{
		reqParamObj = cJSON_GetArrayItem(paramArray, i);
		(*reqObj)->u.setReq->param[i].name = cJSON_GetObjectItem(reqParamObj, "name")->valuestring;
		printf("(*reqObj)->u.setReq->param[%lu].name : %s\n",i,(*reqObj)->u.setReq->param[i].name);
		
		if (cJSON_GetObjectItem(reqParamObj, "value") != NULL )
		{
			(*reqObj)->reqType = SET;
			if(cJSON_GetObjectItem(reqParamObj, "value")->valuestring != NULL && strlen(cJSON_GetObjectItem(reqParamObj, "value")->valuestring) == 0)
			{
				printf("Parameter value is null\n");
			}
			else if(cJSON_GetObjectItem(reqParamObj, "value")->valuestring == NULL)
			{
				printf("Parameter value field is not a string\n");
			}
			else
			{
				(*reqObj)->u.setReq->param[i].value = cJSON_GetObjectItem(reqParamObj, "value")->valuestring;
				printf("(*reqObj)->u.setReq->param[%lu].value : %s\n",i,(*reqObj)->u.setReq->param[i].value);
			}
		}
	
		if (cJSON_GetObjectItem(reqParamObj, "attributes") != NULL )
		{
			attributes = cJSON_GetObjectItem(reqParamObj, "attributes");
			if(cJSON_GetObjectItem(attributes, "notify") != NULL) 
			{
				notification = cJSON_GetObjectItem(attributes, "notify")->valueint;
				printf("notification :%d\n",notification);
				snprintf(notif, sizeof(notif), "%d", notification);
				(*reqObj)->u.setReq->param[i].value = (char *) malloc(sizeof(char) * 20);
				strcpy((*reqObj)->u.setReq->param[i].value, notif);
				printf("(*reqObj)->u.setReq->param[%lu].value : %s\n",i,(*reqObj)->u.setReq->param[i].value);
			}
		}
			
		if (cJSON_GetObjectItem(reqParamObj, "dataType") != NULL)
		{
			(*reqObj)->u.setReq->param[i].type = cJSON_GetObjectItem(reqParamObj, "dataType")->valueint;
			printf("(*reqObj)->u.setReq->param[%lu].type : %d\n",i,(*reqObj)->u.setReq->param[i].type);
		}
	}
			
}

void parse_test_and_set_request(cJSON *request, req_struct **reqObj)
{

	cJSON *reqParamObj = NULL, *paramArray = NULL;	
	size_t paramCount, i;
	
	printf("parsing Test and Set Request\n");
	(*reqObj)->reqType = TEST_AND_SET;
	printf("(*reqObj)->reqType : %d\n",(*reqObj)->reqType);
	
	(*reqObj)->u.testSetReq = (test_set_req_t *) malloc(sizeof(test_set_req_t));
	memset((*reqObj)->u.testSetReq,0,(sizeof(test_set_req_t)));
		
	if(cJSON_GetObjectItem(request, "old-cid") != NULL)
	{
	        (*reqObj)->u.testSetReq->oldCid = cJSON_GetObjectItem(request, "old-cid")->valuestring;
	        printf("(*reqObj)->u.testSetReq->oldCid : %s\n",(*reqObj)->u.testSetReq->oldCid);
	}
	if(cJSON_GetObjectItem(request, "new-cid") != NULL)
	{
	        (*reqObj)->u.testSetReq->newCid = cJSON_GetObjectItem(request, "new-cid")->valuestring;
	        printf("(*reqObj)->u.testSetReq->newCid : %s\n",(*reqObj)->u.testSetReq->newCid);
	}
	if(cJSON_GetObjectItem(request, "sync-cmc") != NULL)
	{
		(*reqObj)->u.testSetReq->syncCmc = cJSON_GetObjectItem(request, "sync-cmc")->valuestring;
		printf("(*reqObj)->u.testSetReq->syncCmc : %s\n",(*reqObj)->u.testSetReq->syncCmc);
	}
		
	if (cJSON_GetObjectItem(request, "parameters") != NULL) // No Parameters
	{
		paramArray = cJSON_GetObjectItem(request, "parameters");
		paramCount = cJSON_GetArraySize(paramArray);
	
		(*reqObj)->u.testSetReq->paramCnt = paramCount;
		printf("(*reqObj)->u.testSetReq->paramCnt : %lu\n",(*reqObj)->u.testSetReq->paramCnt);
	
		(*reqObj)->u.testSetReq->param = (param_t *) malloc(sizeof(param_t) * paramCount);
	
		
		for (i = 0; i < paramCount; i++) 
		{
			reqParamObj = cJSON_GetArrayItem(paramArray, i);
			(*reqObj)->u.testSetReq->param[i].name = cJSON_GetObjectItem(reqParamObj, "name")->valuestring;
			printf("(*reqObj)->u.testSetReq->param[%lu].name : %s\n",i,(*reqObj)->u.testSetReq->param[i].name);
		
			if (cJSON_GetObjectItem(reqParamObj, "value") != NULL)
			{
				(*reqObj)->u.testSetReq->param[i].value = cJSON_GetObjectItem(reqParamObj, "value")->valuestring;
				printf("(*reqObj)->u.testSetReq->param[%lu].value : %s\n",i,(*reqObj)->u.testSetReq->param[i].value);
			
			}
		
			if (cJSON_GetObjectItem(reqParamObj, "dataType") != NULL)
			{
				(*reqObj)->u.testSetReq->param[i].type = cJSON_GetObjectItem(reqParamObj, "dataType")->valueint;
				printf("(*reqObj)->u.testSetReq->param[%lu].type : %d\n",i,(*reqObj)->u.testSetReq->param[i].type);
			}
		}
	}
}
		
void parse_replace_rows_request(cJSON *request, req_struct **reqObj)
{
	
	cJSON *paramArray = NULL, *subitem = NULL;	
	size_t paramCount,rowCnt, i, j;
	
	printf("parsing Replace Rows Request\n");
	(*reqObj)->reqType = REPLACE_ROWS;
	printf("(*reqObj)->reqType : %d\n",(*reqObj)->reqType);
	
	paramArray = cJSON_GetObjectItem(request, "rows");
	rowCnt = cJSON_GetArraySize(paramArray);
	printf("rowCnt : %lu\n",rowCnt);
	
	(*reqObj)->u.tableReq = (table_req_t *) malloc(sizeof(table_req_t));
	memset((*reqObj)->u.tableReq,0,(sizeof(table_req_t)));
	
	(*reqObj)->u.tableReq->rowCnt = rowCnt;
	printf("(*reqObj)->u.tableReq->rowCnt : %lu\n",(*reqObj)->u.tableReq->rowCnt);
	(*reqObj)->u.tableReq->objectName = cJSON_GetObjectItem(request,"table")->valuestring;
	(*reqObj)->u.tableReq->rows = (TableData *) malloc(sizeof(TableData) * rowCnt);
	memset((*reqObj)->u.tableReq->rows,0,(sizeof(TableData) * rowCnt));
	
        for ( i = 0 ; i < rowCnt ; i++)
        {
                subitem = cJSON_GetArrayItem(paramArray, i);
	        paramCount = cJSON_GetArraySize(subitem);
	 	printf("paramCount: %lu\n",paramCount);
	        (*reqObj)->u.tableReq->rows[i].paramCnt = paramCount;
	        printf("(*reqObj)->u.tableReq->rows[%lu].paramCnt : %lu\n",i,(*reqObj)->u.tableReq->rows[i].paramCnt);
	        
	        (*reqObj)->u.tableReq->rows[i].names = (char **) malloc(sizeof(char *) * paramCount);
	        (*reqObj)->u.tableReq->rows[i].values = (char **) malloc(sizeof(char *) * paramCount);
	        for( j = 0 ; j < paramCount ; j++)
	        {
		        (*reqObj)->u.tableReq->rows[i].names[j] = cJSON_GetArrayItem(subitem, j)->string;
		        printf("(*reqObj)->u.tableReq->rows[%lu].names[%lu] : %s\n",i,j,(*reqObj)->u.tableReq->rows[i].names[j]);		
		        		
		        (*reqObj)->u.tableReq->rows[i].values[j] = cJSON_GetArrayItem(subitem, j)->valuestring;
		        printf("(*reqObj)->u.tableReq->rows[%lu].values[%lu] : %s\n",i,j,(*reqObj)->u.tableReq->rows[i].values[j]);	
		        	
	        }
	}
}

void parse_add_row_request(cJSON *request, req_struct **reqObj)
{

	cJSON *paramArray = NULL;	
	size_t paramCount, i;
	
	printf("parsing Add Row Request\n");
	(*reqObj)->reqType = ADD_ROWS;
	printf("(*reqObj)->reqType : %d\n",(*reqObj)->reqType);
	
	paramArray = cJSON_GetObjectItem(request, "row");
	paramCount = cJSON_GetArraySize(paramArray);
	printf("paramCount : %lu\n",paramCount);
	
	(*reqObj)->u.tableReq = (table_req_t *) malloc(sizeof(table_req_t));
	memset((*reqObj)->u.tableReq,0,(sizeof(table_req_t)));
	
	(*reqObj)->u.tableReq->rowCnt = 1;
	printf("(*reqObj)->u.tableReq->rowCnt : %lu\n",(*reqObj)->u.tableReq->rowCnt);
	(*reqObj)->u.tableReq->objectName = cJSON_GetObjectItem(request,"table")->valuestring;
	(*reqObj)->u.tableReq->rows = (TableData *) malloc(sizeof(TableData));
	memset((*reqObj)->u.tableReq->rows,0,(sizeof(TableData)));
	
	(*reqObj)->u.tableReq->rows->names = (char **) malloc(sizeof(char *) * paramCount);
        (*reqObj)->u.tableReq->rows->values = (char **) malloc(sizeof(char *) * paramCount);
        (*reqObj)->u.tableReq->rows->paramCnt = paramCount;
        printf("(*reqObj)->u.tableReq->rows->paramCnt : %lu\n",(*reqObj)->u.tableReq->rows->paramCnt);
        
        for ( i = 0 ; i < paramCount ; i++)
        {
	        (*reqObj)->u.tableReq->rows->names[i] = cJSON_GetArrayItem(paramArray, i)->string;
	         printf("(*reqObj)->u.tableReq->rows->names[%lu] : %s\n",i,(*reqObj)->u.tableReq->rows->names[i]);				
	        (*reqObj)->u.tableReq->rows->values[i] = cJSON_GetArrayItem(paramArray, i)->valuestring;
	        printf("(*reqObj)->u.tableReq->rows->values[%lu] : %s\n",i,(*reqObj)->u.tableReq->rows->values[i]);		
	        	
	}
}

void parse_delete_row_request(cJSON *request, req_struct **reqObj)
{
	
	printf("parsing Delete Row Request\n");
	(*reqObj)->reqType = DELETE_ROW;
	printf("(*reqObj)->reqType : %d\n",(*reqObj)->reqType);
	
	(*reqObj)->u.tableReq = (table_req_t *) malloc(sizeof(table_req_t));
	memset((*reqObj)->u.tableReq,0,(sizeof(table_req_t)));
	(*reqObj)->u.tableReq->objectName = cJSON_GetObjectItem(request,"row")->valuestring;
}

void wdmp_form_get_response(res_struct *resObj, cJSON *response)
{
        cJSON *parameters = NULL,*resParamObj = NULL, *value = NULL, *valueObj = NULL;
        size_t i, paramCount, j;
        char *result = NULL;
        WDMP_RESPONSE_STATUS_CODE statusCode = WDMP_STATUS_GENERAL_FALURE;
        
        printf("resObj->paramCnt : %lu\n",resObj->paramCnt);
        paramCount = resObj->paramCnt;
        printf("paramCount : %lu\n",paramCount);
        result = (char *) malloc(sizeof(char) * MAX_PARAMETER_LEN);
                
        if(resObj->u.getRes)
        {
                
                getStatusCode(&statusCode, paramCount, resObj->retStatus);
                printf("statusCode : %d\n",statusCode);
                if(statusCode == WDMP_STATUS_SUCCESS)
                {
                        cJSON_AddItemToObject(response, "parameters", parameters =cJSON_CreateArray());
                        printf("resObj->u.getRes->paramCnt : %lu\n",resObj->u.getRes->paramCnt);
                        for (i = 0; i < paramCount; i++) 
                        {
                                cJSON_AddItemToArray(parameters, resParamObj = cJSON_CreateObject());
                                printf("resObj->u.getRes->retParamCnt[%lu] : %lu\n",i,resObj->u.getRes->retParamCnt[i]);
                                if(resObj->u.getRes->retParamCnt[i] >= 1)
                                {
		                        if(resObj->u.getRes->retParamCnt[i] > 1)
		                        {
		                                printf("resObj->u.getRes->paramNames[%lu] : %s\n",i,resObj->u.getRes->paramNames[i]);
                                                cJSON_AddStringToObject(resParamObj, "name", resObj->u.getRes->paramNames[i]);
                                                cJSON_AddItemToObject(resParamObj, "value",value = cJSON_CreateArray());
                                                for (j = 0; j < resObj->u.getRes->retParamCnt[i]; j++) 
                                                {
                                                        cJSON_AddItemToArray(value, valueObj = cJSON_CreateObject());
                                                        printf("resObj->u.getRes->params[%lu][%lu].name :%s\n",i,j,resObj->u.getRes->params[i][j].name);
                                                        cJSON_AddStringToObject(valueObj, "name", resObj->u.getRes->params[i][j].name);
		                                        printf("resObj->u.getRes->params[%lu][%lu].value :%s\n",i,j,resObj->u.getRes->params[i][j].value);
		                                        cJSON_AddStringToObject(valueObj, "value",resObj->u.getRes->params[i][0].value);
		                                        printf("resObj->u.getRes->params[%lu][%lu].type :%d\n",i,j,resObj->u.getRes->params[i][j].type);
		                                        cJSON_AddNumberToObject(valueObj, "dataType",resObj->u.getRes->params[i][j].type);
                                                }
                                                cJSON_AddNumberToObject(resParamObj, "dataType",WDMP_NONE);
		                                cJSON_AddNumberToObject(resParamObj, "parameterCount", resObj->u.getRes->retParamCnt[i]);
		                                mapWdmpStatusToStatusMessage(resObj->retStatus[i], result);
		                                cJSON_AddStringToObject(resParamObj, "message", result);
		                        }
		                        else
		                        {
		                                printf("resObj->u.getRes->params[%lu][0].name :%s\n",i,resObj->u.getRes->params[i][0].name);
                                                cJSON_AddStringToObject(resParamObj, "name", resObj->u.getRes->params[i][0].name);
		                                printf("resObj->u.getRes->params[%lu][0].value :%s\n",i,resObj->u.getRes->params[i][0].value);
		                                cJSON_AddStringToObject(resParamObj, "value",resObj->u.getRes->params[i][0].value);
		                                printf("resObj->u.getRes->params[%lu][0].type :%d\n",i,resObj->u.getRes->params[i][0].type);
		                                cJSON_AddNumberToObject(resParamObj, "dataType",resObj->u.getRes->params[i][0].type);
		                                cJSON_AddNumberToObject(resParamObj, "parameterCount", resObj->u.getRes->retParamCnt[i]);
		                                mapWdmpStatusToStatusMessage(resObj->retStatus[i], result);
		                                cJSON_AddStringToObject(resParamObj, "message", result);
		                        }
                                }
                                else
                                {
                                        printf("resObj->u.getRes->paramNames[%lu] : %s\n",i,resObj->u.getRes->paramNames[i]);
                                        cJSON_AddStringToObject(resParamObj, "name", resObj->u.getRes->paramNames[i]);
                                        cJSON_AddStringToObject(resParamObj, "value","EMPTY");
                                        cJSON_AddNumberToObject(resParamObj, "parameterCount", resObj->u.getRes->retParamCnt[i]);
                                }
                        }
                }
                else
                {
                        mapWdmpStatusToStatusMessage(resObj->retStatus[0], result);
		        cJSON_AddStringToObject(response, "message", result);
                }
        }
        
        cJSON_AddNumberToObject(response, "statusCode", statusCode);
        
        if(result)
        {
                free(result); 
        }
}

 void wdmp_form_get_attr_response(res_struct *resObj, cJSON *response)
{
        cJSON *parameters = NULL,*resParamObj = NULL, *attributes = NULL;
        size_t i, paramCount;
        char *result = NULL;
        int notification;
        WDMP_RESPONSE_STATUS_CODE statusCode = WDMP_STATUS_GENERAL_FALURE;
        
        printf("resObj->paramCnt : %lu\n",resObj->paramCnt);
        paramCount = resObj->paramCnt;
        printf("paramCount : %lu\n",paramCount);
                
        if(resObj->u.paramRes)
        {
                
                getStatusCode(&statusCode, paramCount, resObj->retStatus);
                result = (char *) malloc(sizeof(char) * MAX_PARAMETER_LEN);
                if(statusCode == WDMP_STATUS_SUCCESS)
                {
                        cJSON_AddItemToObject(response, "parameters", parameters =cJSON_CreateArray());
                        for (i = 0; i < paramCount; i++) 
                        {
                                cJSON_AddItemToArray(parameters, resParamObj = cJSON_CreateObject());
                                printf("resObj->u.paramRes->params[%lu].name :%s\n",i,resObj->u.paramRes->params[i].name);
                                cJSON_AddStringToObject(resParamObj, "name", resObj->u.paramRes->params[i].name);
                                cJSON_AddItemToObject(resParamObj, "attributes",attributes = cJSON_CreateObject());
                                printf("resObj->u.paramRes->params[%lu].value :%s\n",i,resObj->u.paramRes->params[i].value);
		                notification = atoi(resObj->u.paramRes->params[i].value);
		                printf("notification : %d\n", notification);
		                cJSON_AddNumberToObject(attributes, "notify", notification);
		                printf("resObj->retStatus[i] :%d\n",resObj->retStatus[i]);
                                mapWdmpStatusToStatusMessage(resObj->retStatus[i], result);
                                cJSON_AddStringToObject(resParamObj, "message", result);
                        }
                }
                else
                {
                        mapWdmpStatusToStatusMessage(resObj->retStatus[0], result);
		        cJSON_AddStringToObject(response, "message", result);
                }
                
                if(result)
                {
                        free(result); 
                }
        }
        
        printf("statusCode : %d\n",statusCode);
        cJSON_AddNumberToObject(response, "statusCode", statusCode);
        
}

 void wdmp_form_set_response(res_struct *resObj, cJSON *response)
{
        cJSON *parameters = NULL,*resParamObj = NULL;
        size_t i, paramCount;
        char *result = NULL;
        WDMP_RESPONSE_STATUS_CODE statusCode = WDMP_STATUS_GENERAL_FALURE;
        
        printf("resObj->paramCnt : %lu\n",resObj->paramCnt);
        paramCount = resObj->paramCnt;
        printf("paramCount : %lu\n",paramCount);  
        
        printf("resObj->retStatus : %d\n",resObj->retStatus[0]);
        getStatusCode(&statusCode, paramCount, resObj->retStatus);
        
        result = (char *) malloc(sizeof(char) * MAX_PARAMETER_LEN);
        
        if(resObj->u.paramRes->params)
        {
                cJSON_AddItemToObject(response, "parameters", parameters =cJSON_CreateArray());
                
                for (i = 0; i < paramCount; i++) 
                {
                        cJSON_AddItemToArray(parameters, resParamObj = cJSON_CreateObject());
                        
                        printf("resObj->u.paramRes->params[%lu].name :%s\n",i,resObj->u.paramRes->params[i].name);
                        cJSON_AddStringToObject(resParamObj, "name", resObj->u.paramRes->params[i].name);
                        
                        printf("resObj->retStatus[%lu] : %d\n",i,resObj->retStatus[i]);
                        mapWdmpStatusToStatusMessage(resObj->retStatus[i], result);
                        cJSON_AddStringToObject(resParamObj, "message", result);
                }
                
        }
        else
        {
                mapWdmpStatusToStatusMessage(resObj->retStatus[0], result);
	        cJSON_AddStringToObject(response, "message", result);
        }
        
        if(result)
        {
                free(result); 
        }
        printf("statusCode : %d\n",statusCode);
        cJSON_AddNumberToObject(response, "statusCode", statusCode);
}

void wdmp_form_table_response(res_struct *resObj, cJSON *response)
{
        char *result = NULL;
        WDMP_RESPONSE_STATUS_CODE statusCode = WDMP_STATUS_GENERAL_FALURE;
        
        result = (char *) malloc(sizeof(char) * MAX_PARAMETER_LEN);
        
        printf("resObj->retStatus : %d\n",resObj->retStatus[0]);
        getStatusCode(&statusCode,1,resObj->retStatus);
        
        if(resObj->u.tableRes)
        {
                printf("resObj->u.tableRes->newObj : %s\n",resObj->u.tableRes->newObj);
                cJSON_AddStringToObject(response, "row", resObj->u.tableRes->newObj);
                if(statusCode == WDMP_STATUS_SUCCESS)
                {
                       statusCode = WDMP_ADDROW_STATUS_SUCCESS; 
                }
        }
        
        mapWdmpStatusToStatusMessage(resObj->retStatus[0], result);
        printf("result : %s\n",result);
        cJSON_AddStringToObject(response, "message", result);
        
        if(result)
        {
                free(result); 
        }
        
        printf("statusCode :%d\n",statusCode);
        cJSON_AddNumberToObject(response, "statusCode", statusCode);
}

 void wdmp_form_test_and_set_response(res_struct *resObj, cJSON *response)
{
        cJSON *parameters = NULL,*resParamObj1 = NULL, *resParamObj2 = NULL;
        size_t paramCount;
        char *result = NULL;
        WDMP_RESPONSE_STATUS_CODE statusCode = WDMP_STATUS_GENERAL_FALURE;
        
        printf("resObj->paramCnt : %lu\n",resObj->paramCnt);
        paramCount = resObj->paramCnt;
        printf("paramCount : %lu\n",paramCount);
        
        printf("resObj->retStatus : %d\n",resObj->retStatus[0]);
        getStatusCode(&statusCode, 1, resObj->retStatus);
                
        result = (char *) malloc(sizeof(char) * MAX_PARAMETER_LEN);
                        
        if(resObj->u.paramRes)
        {
                
                if((NULL != resObj->u.paramRes->syncCMC) && (NULL != resObj->u.paramRes->syncCID))
                {
                        cJSON_AddItemToObject(response, "parameters", parameters =cJSON_CreateArray());
                        cJSON_AddItemToArray(parameters, resParamObj1 = cJSON_CreateObject());
		        cJSON_AddStringToObject(resParamObj1, "name", WDMP_SYNC_PARAM_CID);
		        cJSON_AddStringToObject(resParamObj1, "value", resObj->u.paramRes->syncCID);
		        cJSON_AddItemToArray(parameters, resParamObj2 = cJSON_CreateObject());
		        cJSON_AddStringToObject(resParamObj2, "name", WDMP_SYNC_PARAM_CMC);
		        cJSON_AddNumberToObject(resParamObj2, "value", atoi(resObj->u.paramRes->syncCMC));
                }
        }
        
        mapWdmpStatusToStatusMessage(resObj->retStatus[0], result);
        cJSON_AddStringToObject(response, "message", result);
        
        if(result)
        {
                free(result); 
        }
        
        printf("statusCode : %d\n",statusCode);
        cJSON_AddNumberToObject(response, "statusCode", statusCode);
        
}

 void mapWdmpStatusToStatusMessage(WDMP_STATUS status, char *result) 
{
	if (status == WDMP_SUCCESS) 
	{ 
		strcpy(result,"Success");
	} 
	else if (status == WDMP_ERR_INVALID_PARAMETER_NAME) 
	{
		strcpy(result, "Invalid parameter name");
	} 
	else if (status == WDMP_ERR_INVALID_PARAMETER_TYPE) 
	{
		strcpy(result,"Invalid parameter type");
	}
	else if (status == WDMP_ERR_INVALID_PARAMETER_VALUE) 
	{
		strcpy(result,"Invalid parameter value");
	} 
	else if (status == WDMP_ERR_NOT_WRITABLE) 
	{
		strcpy(result,"Parameter is not writable");
	}
	else if (status == WDMP_ERR_NOT_EXIST) 
	{
		strcpy(result,"Parameter does not exist");
	} 
	else if (status == WDMP_FAILURE) 
	{
		strcpy(result,"Failure");
	} 
	else if (status == WDMP_ERR_TIMEOUT) 
	{
		strcpy(result,"Error Timeout");
	}
	else if (status == WDMP_ERR_SETATTRIBUTE_REJECTED) 
	{
		strcpy(result,"SetAttribute rejected");
	} 
	else if (status == WDMP_ERR_NAMESPACE_OVERLAP) 
	{
		strcpy(result,"Error namespace overlap");
	} 
	else if (status == WDMP_ERR_UNKNOWN_COMPONENT) 
	{
		strcpy(result,"Error unkown component");
	} 
	else if (status == WDMP_ERR_NAMESPACE_MISMATCH) 
	{
		strcpy(result,"Error namespace mismatch");
	} 
	else if (status == WDMP_ERR_UNSUPPORTED_NAMESPACE) 
	{
		strcpy(result,"Error unsupported namespace");
	} 
	else if (status == WDMP_ERR_DP_COMPONENT_VERSION_MISMATCH) 
	{
		strcpy(result,"Error component version mismatch");
	} 
	else if (status == WDMP_ERR_INVALID_PARAM) 
	{
		strcpy(result,"Invalid Param");
	}
	else if (status == WDMP_ERR_UNSUPPORTED_DATATYPE) 
	{
		strcpy(result,"Unsupported datatype");
	}
	else if (status == WDMP_ERR_WIFI_BUSY) 
	{
		strcpy(result,"WiFi is busy");
	}
	else if (status == WDMP_ERR_INVALID_ATTRIBUTES)
	{
	        strcpy(result,"Invalid attributes");
	}
	else if (status == WDMP_ERR_WILDCARD_NOT_SUPPORTED)
	{
	        strcpy(result,"Wildcard is not supported");
	}
	else if (status == WDMP_ERR_SET_OF_CMC_OR_CID_NOT_SUPPORTED)
	{
	        strcpy(result,"SET of CMC or CID is not supported");
	}
	else if (status == WDMP_ERR_VALUE_IS_EMPTY)
	{
	        strcpy(result,"Parameter value field is not available");
	}
	else if (status == WDMP_ERR_VALUE_IS_NULL)
	{
	        strcpy(result,"Parameter value is null");
	}
	else if (status == WDMP_ERR_DATATYPE_IS_NULL)
	{
	        strcpy(result,"Parameter dataType is null");
	}
	else if (status == WDMP_ERR_CMC_TEST_FAILED)
	{
	        strcpy(result,"CMC test failed");
	}
	else if (status == WDMP_ERR_NEW_CID_IS_MISSING)
	{
	        strcpy(result,"New-Cid is missing");
	}
	else if (status == WDMP_ERR_ATTRIBUTES_IS_NULL)
	{
	        strcpy(result,"attributes is null");
	}
	else if (status == WDMP_ERR_NOTIFY_IS_NULL)
	{
	        strcpy(result,"notify is null");
	}
	else if (status == WDMP_ERR_CID_TEST_FAILED)
	{
	        strcpy(result,"CID test failed");
	}
	else if (status == WDMP_ERR_ATOMIC_GET_SET_FAILED)
	{
	        strcpy(result,"Atomic Set failed");
	}
	else if (status == WDMP_ERR_SETTING_CMC_OR_CID)
	{
	        strcpy(result,"Error setting CID/CMC");
	}
	else if (status == WDMP_ERR_INVALID_INPUT_PARAMETER)
	{
	        strcpy(result,"Invalid Input parameter - CID/CMC value cannot be set");
	}
	else 
	{
		strcpy(result,"Unknown Error");
	}
}

 void getStatusCode(WDMP_RESPONSE_STATUS_CODE *statusCode, int paramCount, WDMP_STATUS * ret)
{
	int i =0;
	for (i = 0; i < paramCount; i++) 
	{
		printf("ret[%d] = %d\n",i,ret[i]);
		if (ret[i] == WDMP_SUCCESS) 
		{
			*statusCode = WDMP_STATUS_SUCCESS;
		}
		else if (ret[i] == WDMP_ERR_WIFI_BUSY)
		{
			*statusCode = WDMP_STATUS_WIFI_BUSY;
			break;
		}
		else if (ret[i] == WDMP_ERR_CMC_TEST_FAILED)
		{
			*statusCode = WDMP_STATUS_CMC_TEST_FAILED;
			break;
		}
		else if (ret[i] == WDMP_ERR_CID_TEST_FAILED || ret[i] == WDMP_ERR_NEW_CID_IS_MISSING)
		{
			*statusCode = WDMP_STATUS_CID_TEST_FAILED;
			break;
		}
		else if (ret[i] == WDMP_ERR_ATOMIC_GET_SET_FAILED || ret[i] == WDMP_ERR_WILDCARD_NOT_SUPPORTED || ret[i] == WDMP_ERR_SET_OF_CMC_OR_CID_NOT_SUPPORTED || ret[i] == WDMP_ERR_VALUE_IS_EMPTY || ret[i] == WDMP_ERR_VALUE_IS_NULL || ret[i] == WDMP_ERR_DATATYPE_IS_NULL || ret[i] == WDMP_ERR_NOTIFY_IS_NULL || ret[i] == WDMP_ERR_ATTRIBUTES_IS_NULL || ret[i] == WDMP_ERR_SETTING_CMC_OR_CID)
		{
			*statusCode = WDMP_STATUS_ATOMIC_GET_SET_FAILED;
			break;
		}
		else 
		{
			*statusCode = WDMP_STATUS_GENERAL_FALURE;
			break;
		}
	}
	printf("*statusCode = %d\n",*statusCode);
}
/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/* none */