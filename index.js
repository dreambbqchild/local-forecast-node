require('dotenv').config(); //Set OPENWEATHERMAP_APPID to your OpenWeatherMap AppId.
var localForeast = require('./build/Release/local-forecast-node');

(async () => {
    await localForeast.renderRegionalForecast();
    console.log('something');
})();