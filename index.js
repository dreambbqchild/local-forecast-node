require('dotenv').config(); //Set OPENWEATHERMAP_APPID to your OpenWeatherMap AppId.
var localForeast = require('./build/Release/local-forecast-node');

(async () => {
    //console.log(await localForeast.renderRegionalForecast("t-c"));
    console.log(await localForeast.preprocessHRRRForCurrentValidPeriod("t-c"));
    console.log(await localForeast.renderPreprocessedHRRRForNow("t-c"));
    console.log(await localForeast.renderFullLocalHRRRForecast("t-c"));
})();