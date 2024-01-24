#include "promises.h"

#include <assert.h>
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>

#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

static inline char* ReadUTF8String(napi_env env, napi_value value)
{
  size_t length = 0;
  napi_get_value_string_utf8(env, value, NULL, 0, &length);
  length += 1;

  char * str = (char*)calloc(length, sizeof(char));
  napi_get_value_string_utf8(env, value, str, length, &length);
  return str;
}

static bool LoadArgumentsWithValidation(napi_env env, napi_callback_info info, size_t expected, napi_value* args, napi_valuetype* expectedTypes)
{
  size_t argc = expected;
  napi_status status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
  if(argc != expected)
    return false;

  for(size_t i = 0; i < expected; i++)
  {
    napi_valuetype actualType;
    status = napi_typeof(env, args[i], &actualType);
    assert(status == napi_ok);
    
    if(expectedTypes[i] != actualType)
      return false;
  }

  return true;
}

static void RenderLocalForecastCallback(napi_env env, void *data)
{
  PromiseData* promiseData = (PromiseData*)data;
  switch(promiseData->promiseType)
  {
    case RegionalForecastPromiseType: LocalForecastLibRenderRegionalForecast(promiseData->locationKey, &promiseData->regionalImagePath); return;
    case HRRRForecastPromiseType: LocalForecastLibRenderLocalForecast(promiseData->locationKey, HRRRWxModel, promiseData->renderTargets, 1, 48, &promiseData->videoPath, &promiseData->textPath); return;
    case CachedHRRRForecastPromiseType: LocalForecastLibRenderCahcedLocalForecast(promiseData->locationKey, HRRRWxModel, promiseData->renderTargets, &promiseData->videoPath, &promiseData->textPath); return;    
    default:
      fputs("Unknown Promise Type\n", stderr);
  }    
}

static napi_value StartPromise(napi_env env, napi_callback_info info, const char* name, enum RenderTargets renderTargets, enum PromiseType promiseType)
{  
  napi_value args[1] = {0};
  napi_valuetype types[] = { napi_string };
  if(!LoadArgumentsWithValidation(env, info, 1, (napi_value*)args, (napi_valuetype*)types))
    return CreateRejectPromise(env, "Needs 1 parameter, and it must be a string.");

  PromiseData* promiseData = (PromiseData*)calloc(1, sizeof(PromiseData));
  promiseData->promiseType = promiseType;
  promiseData->renderTargets = renderTargets;

  promiseData->locationKey = ReadUTF8String(env, args[0]);

  return CreatePromise(env, name, promiseData, RenderLocalForecastCallback);
}

//Renders the Regional forecast for right now.
static napi_value RenderRegionalForecast(napi_env env, napi_callback_info info) 
{  
  //The render target on this one is ignored.
  return StartPromise(env, info, "RenderRegionalForecast", RegionalForecastRenderTarget, RegionalForecastPromiseType);
}

//Does the time consuming work: Downloading the GRIB file and rendering the weather maps.
static napi_value PreprocessHRRRForCurrentValidPeriod(napi_env env, napi_callback_info info)
{
  return StartPromise(env, info, "PreprocessHRRRForCurrentValidPeriod", WeatherMapsRenderTarget, HRRRForecastPromiseType);
}

//Uses the GRIBs and WeatherMap pngs cached locally at the supplied location to render the forecast for now.
static napi_value RenderPreprocessedHRRRForNow(napi_env env, napi_callback_info info)
{
  return StartPromise(env, info, "RenderPreprocessedHRRRForNow", RegionalForecastRenderTarget | PersonalForecastsRenderTarget | TextForecastRenderTarget | VideoRenderTarget, CachedHRRRForecastPromiseType);
}

//Renders EVERYTHING.
static napi_value RenderFullLocalHRRRForecast(napi_env env, napi_callback_info info) 
{ 
  return StartPromise(env, info, "RenderFullLocalHRRRForecast", AllRenderTargets, HRRRForecastPromiseType);
}

static napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
    DECLARE_NAPI_METHOD("renderRegionalForecast", RenderRegionalForecast),
    DECLARE_NAPI_METHOD("preprocessHRRRForCurrentValidPeriod", PreprocessHRRRForCurrentValidPeriod),
    DECLARE_NAPI_METHOD("renderPreprocessedHRRRForNow", RenderPreprocessedHRRRForNow),
    DECLARE_NAPI_METHOD("renderFullLocalHRRRForecast", RenderFullLocalHRRRForecast)
  }; 

  napi_status status = napi_define_properties(env, exports, 4, desc);

  assert(status == napi_ok);

  LocalForecastLibInit();

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)