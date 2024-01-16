require('dotenv').config(); //Set OPENWEATHERMAP_APPID to your OpenWeatherMap AppId.
var localForeast = require('bindings')('local-forecast-node');

(async () => {
    await localForeast.renderRegionalForecast();
    console.log('something');
})();