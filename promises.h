#ifndef PROMISES_H
#define PROMISES_H

#include <LocalForecastLib.h>
#include <node_api.h>

enum PromiseType {
  RegionalForecastPromiseType,
  HRRRForecastPromiseType,
  CachedHRRRForecastPromiseType
};

typedef struct {
  enum PromiseType promiseType;
  enum RenderTargets renderTargets;
  char* locationKey, 
      * regionalImageFilePath,
      * videoFilePath, 
      * textFilePath;

  napi_async_work work;
  napi_deferred deferred;
} PromiseData;

napi_value CreatePromise(napi_env env, const char* name, PromiseData* promiseData, napi_async_execute_callback executeCallback);
napi_value CreateRejectPromise(napi_env env, const char* message);

#endif