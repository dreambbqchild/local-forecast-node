#include "promises.h"
#include <stdlib.h>

void CompletePromise(napi_env env, napi_status status, void *data)
{
  napi_value result;
  PromiseData *promiseData = (PromiseData *)data;

  napi_create_int32(env, 1, &result);
  napi_resolve_deferred(env, promiseData->deferred, result);

  //Begin all important cleanup.
  napi_delete_async_work(env, promiseData->work);
  
  if(promiseData->gribFilePath)
  {
    free(promiseData->gribFilePath);
    promiseData->gribFilePath = NULL;
  }
  
  if(promiseData->forecastFilePath)
  {
    free(promiseData->forecastFilePath);
    promiseData->forecastFilePath = NULL;
  }

  free(promiseData);
}

napi_value CreatePromise(napi_env env, const char* name, PromiseData* promiseData, napi_async_execute_callback executeCallback)
{
  napi_value promise;
  napi_value promiseName;

  napi_create_promise(env, &promiseData->deferred, &promise);
  napi_create_string_utf8(env, name, -1, &promiseName);
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