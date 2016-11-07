var compassPoints = require('./compassPoints');

var OpenWeatherMapsAPIKey = "4de2ed41f0012eda52964daee1a58d70";
var FawknerBeaconURL = "http://reg.bom.gov.au/fwo/IDV60901/IDV60901.95872.json";
//var WindSource = 'OWM';
var WindSource = 'BOM';

var BOMStationURL = FawknerBeaconURL;

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function messageFailedCallback(e) {
    console.log('Error sending info to Pebble!');
}

function sendWeatherToPebble(dictionary) {
    // Send to Pebble
    Pebble.sendAppMessage(dictionary, function(e) {console.log('Weather info sent to Pebble successfully.');}, messageFailedCallback);
}

function updateWeatherFromOWM(pos) {
    // Construct URL
    var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
    pos.coords.latitude + '&lon=' + pos.coords.longitude + '&units=metric' + '&appid=' + OpenWeatherMapsAPIKey;
    
    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET',
               function(responseText) {
                    // console.log('Weather response:' + responseText);
                    // responseText contains a JSON object with weather info
                    var json = JSON.parse(responseText);

                    // Get weather data and send to Pebble
                    var temperature = Math.round(json.main.temp);
                    var conditions = json.weather[0].main;
               
                    var dictionary = {
                        'TEMPERATURE': temperature,
                        'CONDITIONS': conditions
                    };

                   if(WindSource == 'OWM') {
                        // Get wind data and send to Pebble
                        var wind_speed = Math.round(json.wind.speed * 1.943844);
                        var wind_direction = compassPoints.azimuthToAbbreviation(json.wind.deg);
                        console.log('Wind direction converted ' + json.wind.deg + ' to ' + wind_direction);
               
                        dictionary = {
                            'TEMPERATURE': temperature,
                            'CONDITIONS': conditions,
                            'WIND_SPEED': wind_speed,
                            'WIND_DIRECTION': wind_direction
                        };
                    }
                    sendWeatherToPebble(dictionary);
               }
               );
}

function updateWeatherFromBOM(pos) {
    if(WindSource != 'BOM')
        return;
    
    // Construct URL
    var url = BOMStationURL;
    
    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET',
               function(responseText) {
                   console.log('Got response from BOM, parsing ...');
                   // responseText contains a JSON object with weather info
                   var json = JSON.parse(responseText);
                   
                   // Get wind data and send to Pebble
                   var wind_speed = json.observations.data[0].wind_spd_kt;
                   var wind_direction = json.observations.data[0].wind_dir;

                   var dictionary = {
                        'WIND_SPEED': wind_speed,
                        'WIND_DIRECTION': wind_direction
                   };
               
                    console.log('Wind data to send:' + wind_speed + ',' + wind_direction);
                   sendWeatherToPebble(dictionary);
               }
               );
}

function locationSuccess(pos) {
    // We will request the weather here
    console.log('We got location:' + pos);

    updateWeatherFromOWM(pos);
    updateWeatherFromBOM(pos);
}

function locationError(err) {
    console.log('Error requesting location!');
}

function updateWeather() {
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
        updateWeather();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function(e) {
        console.log('AppMessage received, updating weather...');
        updateWeather();
    }
);
