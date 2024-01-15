require('dotenv').config(); //Set OPENWEATHERMAP_APPID to your OpenWeatherMap AppId.
var localForeast = require('bindings')('local-forecast-node');

localForeast.preprocessHRRRForCurrentValidPeriod('./data/hrrr', './forecasts/hrrr');