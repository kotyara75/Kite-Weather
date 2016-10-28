var myAPIKey = "4de2ed41f0012eda52964daee1a58d70";

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function locationSuccess(pos) {
    // We will request the weather here
    console.log('We got location:' + pos);
    
    // Construct URL
    var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
    pos.coords.latitude + '&lon=' + pos.coords.longitude + '&units=metric' + '&appid=' + myAPIKey;
    
    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET',
       function(responseText) {
               console.log('Weather response:' + responseText);
               // responseText contains a JSON object with weather info
               var json = JSON.parse(responseText);
               
               // Temperature in Kelvin requires adjustment
               var temperature = Math.round(json.main.temp);
               
               // Conditions
               var conditions = json.weather[0].main;
               
               // Wind Speed
               var wind_speed = Math.round(json.wind.speed * 1.943844);
               
               // Wind Direction
               var wind_direction = Math.round(json.wind.deg);
               
               // Assemble dictionary using our keys
               var dictionary = {
                   'TEMPERATURE': temperature,
                   'CONDITIONS': conditions,
                    'WIND_SPEED': wind_speed,
                    'WIND_DIRECTION': wind_direction
               };
               
               // Send to Pebble
               Pebble.sendAppMessage(dictionary,
                 function(e) {
                     console.log('Weather info sent to Pebble successfully.');
                 },
                 function(e) {
                     console.log('Error sending weather info to Pebble!');
                 }
             );
       }
   );
}

function locationError(err) {
    console.log('Error requesting location!');
}

function getWeather() {
    console.log('Getting location ...');
    navigator.geolocation.getCurrentPosition(
        locationSuccess,
        locationError,
        {timeout: 15000, maximumAge: 60000}
    );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
    function(e) {
        console.log('PebbleKit JS ready.');

        // Get the initial weather
        getWeather();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function(e) {
        console.log('AppMessage received, get weather...');
        getWeather();
    }
);
