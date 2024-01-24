#include "promises.h"
#include <stdlib.h>

inline void AddStringPropertyToObject(napi_env env, const char* rawString, const char* propertyName, napi_value obj)
{
  if(!rawString)
    return;
  
  napi_value strValue = NULL;
  napi_create_string_utf8(env, rawString, NAPI_AUTO_LENGTH, &strValue);
  napi_set_named_property(env, obj, propertyName, strValue);
}

void CompletePromise(napi_env env, napi_status status, void *data)
{
  napi_value result;
  PromiseData *promiseData = (PromiseData *)data;

  napi_create_object(env, &result);
  AddStringPropertyToObject(env, promiseData->regionalImagePath, "regionalForecastPath", result);
  AddStringPropertyToObject(env, promiseData->videoPath, "videoPath", result);  
  AddStringPropertyToObject(env, promiseData->textPath, "textPath", result);  

  napi_resolve_deferred(env, promiseData->deferred, result);

  //Begin all important cleanup.
  napi_delete_async_work(env, promiseData->work);
  
  if(promiseData->locationKey)
  {
    free(promiseData->locationKey);
    promiseData->locationKey = NULL;
  }

  if(promiseData->regionalImagePath)
  {
    free(promiseData->regionalImagePath);
    promiseData->regionalImagePath = NULL;
  }

  if(promiseData->videoPath)
  {
    free(promiseData->videoPath);
    promiseData->videoPath = NULL;
  }
  
  if(promiseData->textPath)
  {
    free(promiseData->textPath);
    promiseData->textPath = NULL;
  }

  free(promiseData);
}

napi_value CreatePromise(napi_env env, const char* name, PromiseData* promiseData, napi_async_execute_callback executeCallback)
{
  napi_value promise;
  napi_value promiseName;

  napi_create_promise(env, &promiseData->deferred, &promise);
  napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &promiseName);
  napi_create_async_work(env, NULL, promiseName, executeCallback, CompletePromise, (void*)promiseData, &promiseData->work);
  napi_queue_async_work(env, promiseData->work);
  return promise;
}

napi_value CreateRejectPromise(napi_env env, const char* message)
{
    napi_value promise;
    napi_deferred deferred;
    napi_value napiMessage;        
    napi_create_promise(env, &deferred, &promise);

    napi_create_string_utf8(env, message, NAPI_AUTO_LENGTH, &napiMessage);
    napi_reject_deferred(env, deferred, napiMessage);
    return promise;
}