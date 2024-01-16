#include "promises.h"

#include <assert.h>
#include <stdbool.h> 
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
  if(promiseData->useCache)
    LocalForecastLibRenderCahcedLocalForecast(promiseData->gribFilePath, promiseData->forecastFilePath, promiseData->renderTargets);
  else
    LocalForecastLibRenderLocalForecast(HRRRWxModel, promiseData->gribFilePath, promiseData->forecastFilePath, promiseData->renderTargets, 1, 48);
}

static void RenderRegionalForecastCallback(napi_env env, void* data) 
{  
  LocalForecastLibRenderRegionalForecast();
}

static napi_value ProcessHRRRForecasts(napi_env env, napi_callback_info info, const char* name, enum RenderTargets renderTargets, bool useCache)
{  
  napi_value args[2] = {0};
  napi_valuetype types[] = { napi_string, napi_string };
  if(!LoadArgumentsWithValidation(env, info, 2, (napi_value*)args, (napi_valuetype*)types))
    return CreateRejectPromise(env, "Needs 2 parameters, and they must be strings.");

  PromiseData* promiseData = (PromiseData*)calloc(1, sizeof(PromiseData));
  promiseData->useCache = useCache;
  promiseData->renderTargets = renderTargets;

  promiseData->gribFilePath = ReadUTF8String(env, args[0]);
  promiseData->forecastFilePath = ReadUTF8String(env, args[1]);

  return CreatePromise(env, name, promiseData, RenderLocalForecastCallback);
}

//Renders the Regional forecast for right now.
static napi_value RenderRegionalForecast(napi_env env, napi_callback_info info) 
{  
  return CreatePromise(env, "RenderRegionalForecast", (PromiseData*)calloc(1, sizeof(PromiseData)), RenderRegionalForecastCallback);
}

//Does the time consuming work: Downloading the GRIB file and rendering the weather maps.
static napi_value PreprocessHRRRForCurrentValidPeriod(napi_env env, napi_callback_info info)
{
  return ProcessHRRRForecasts(env, info, "PreprocessHRRRForCurrentValidPeriod", WeatherMapsRenderTarget, false);
}

//Uses the GRIBs and WeatherMap pngs cached locally at the supplied location to render the forecast for now.
static napi_value RenderPreprocessedHRRRForNow(napi_env env, napi_callback_info info)
{
  return ProcessHRRRForecasts(env, info, "RenderPreprocessedHRRRForNow", RegionalForecastRenderTarget | PersonalForecastsRenderTarget | TextForecastRenderTarget | VideoRenderTarget, true);
}

//Renders EVERYTHING.
static napi_value RenderFullLocalHRRRForecast(napi_env env, napi_callback_info info) 
{ 
  return ProcessHRRRForecasts(env, info, "RenderFullLocalHRRRForecast", AllRenderTargets, false);
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