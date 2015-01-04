var isReady = false;
var isFetching = false;
var callbacks = [];
var curTime;

var weather = {
  off: 0,
  on_15: 15,
  on_30: 30,
  on_60: 59
};


var locationOptions = { "timeout": 150000, "maximumAge": 600000 };

/*
function fetchWeather(latitude, longitude) {
  curTime = Math.floor((new Date).getTime()/1000);
  var lastFetch = localStorage.getItem("lastFetch");

  if (isFetching) {
    console.log("fetchWeather: already fetching, quit");
    return;
  }
  else if (curTime - lastFetch < 900) {
    var icon = localStorage.getItem("icon");
    var temperatureC = localStorage.getItem("temperatureC");
    transmitConfiguration({
      "icon":icon,
      "temperatureC":"" + temperatureC+"\u00B0C"
      });
    console.log("fetchWeather: already fetched recently, quit"+icon+temperatureC);
    return;
  }
  else {
    isFetching = true;
    console.log("fetchWeather: fetching now");

    var response;
    var req = new XMLHttpRequest();
    req.open('GET', "http://api.openweathermap.org/data/2.5/find?" +
               "lat=" + latitude + "&lon=" + longitude + "&cnt=1", true);
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200) {
          console.log(req.responseText);
          response = JSON.parse(req.responseText);
          var temperatureC, icon, city;
          if (response && response.list && response.list.length > 0) {
            var weatherResult = response.list[0];
            temperatureC = Math.round(weatherResult.main.temp - 273.15);
            icon = weatherResult.weather[0].main;
            city = weatherResult.name;
            console.log(temperatureC);
            console.log(icon);
            console.log(city);
            localStorage.setItem("lastFetch", curTime);
            localStorage.setItem("icon", icon);
            localStorage.setItem("temperatureC", temperatureC);
            transmitConfiguration({
              "icon":icon,
              "temperatureC":"" + temperatureC+"\u00B0C"
              });
          }
        } else {
          console.log("Error");
        }
      }
    };
    req.send(null);
    isFetching = false;
    console.log("fetchWeather: finished fetching, quit");
  }
}

*/

function fetchWeather(latitude, longitude) {
  curTime = Math.floor((new Date).getTime()/1000);
  var lastFetch = localStorage.getItem("lastFetch");

  if (isFetching) {
    console.log("fetchWeather: already fetching, quit");
    return;
  }
  else if (curTime - lastFetch < 900) {
    var icon = localStorage.getItem("icon");
    var temperatureC = localStorage.getItem("temperatureC");
    transmitConfiguration({
      "icon":icon,
      "temperatureC":"" + temperatureC
      });
    console.log("fetchWeather: already fetched recently, quit"+icon+temperatureC);
    return;
  }
  else {
    isFetching = true;
    console.log("fetchWeather: fetching now");

  var response;
  var woeid = -1;
  var query = encodeURI("select woeid from geo.placefinder where text=\""+latitude+","+longitude + "\" and gflags=\"R\"");
  var url = "http://query.yahooapis.com/v1/public/yql?q=" + query + "&format=json";
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        response = JSON.parse(req.responseText);
        if (response) {
          woeid = response.query.results.Result.woeid;
          getWeatherFromWoeid(woeid);
        }
      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
  }
}

function getWeatherFromWoeid(woeid) {
  var celsius = 'celsius';
  var query = encodeURI("select item.condition from weather.forecast where woeid = " + woeid +
                        " and u = " + (celsius ? "\"c\"" : "\"f\""));
  var url = "http://query.yahooapis.com/v1/public/yql?q=" + query + "&format=json";

  var response;
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        response = JSON.parse(req.responseText);
        if (response) {
          console.log(req.responseText);
          var condition = response.query.results.channel.item.condition;
          temperature = condition.temp + (celsius ? "\u00B0C" : "\u00B0F");
          icon = condition.text;
          localStorage.setItem("lastFetch", curTime);
          localStorage.setItem("icon", icon);
          localStorage.setItem("temperatureC", temperature);
          transmitConfiguration({
            "icon":icon,
            "temperatureC":"" + temperature
          });
        }
      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
    isFetching = false;
    console.log("fetchWeather: finished fetching, quit");
}



function locationSuccess(pos) {
  var coordinates = pos.coords;
  var datetime = "======= lastsync: " + new Date();
  console.log(datetime);
  if(!isFetching)fetchWeather(coordinates.latitude, coordinates.longitude);
  //window.navigator.geolocation.clearWatch(locationWatcher);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  transmitConfiguration({
    "icon":"no data",
    "temperatureC":"     "
    });
}

function readyCallback(event) {
  isReady = true;
  var callback;
  while (callbacks.length > 0) {
    callback = callbacks.shift();
    callback(event);
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  }
}

// Retrieves stored configuration from localStorage.
function getOptions() {
  return localStorage.getItem("options") || ("{}");
}

// Stores options in localStorage.
function setOptions(options) {
  localStorage.setItem("options", options);
}

// Takes a string containing serialized JSON as input.  This is the
// format that is sent back from the configuration web UI.  Produces
// a JSON message to send to the watch face.
function prepareConfiguration(serialized_settings) {
  var settings = JSON.parse(serialized_settings);
  return {
    "1": settings.bluetooth ? 1 : 0,
    "2": weather[settings.weather]
  };
}

// Takes a JSON message as input.  Sends the message to the watch.
function transmitConfiguration(settings) {
  //console.log('sending message: '+ JSON.stringify(settings));
  Pebble.sendAppMessage(settings, function(event) {
    // Message delivered successfully
  }, logError);
}

function logError(event) {
  console.log('Unable to deliver message with transactionId='+
              event.data.transactionId +' ; Error is'+ event.error.message);
}


function showConfiguration(event) {
  //onReady(function() {
    var opts = getOptions();
    var url  = "http://zecoj.github.io/digitZ/";
    console.log(opts);
    Pebble.openURL(url + "#options=" + encodeURIComponent(opts));
  //});
}

function webviewclosed(event) {
  var resp = event.response;
  console.log('configuration response: '+ resp + ' ('+ typeof resp +')');

  var options = JSON.parse(resp);
  if (typeof options.bluetooth === 'undefined' &&
      typeof options.weather === 'undefined') {
    return;
  }

  onReady(function() {
    setOptions(resp);

    var message = prepareConfiguration(resp);
    transmitConfiguration(message);
  });
}

function appmessage(event) {
  if(!isFetching)window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  console.log("message!");
}

function onReady(callback) {
  if (isReady) {
    callback();
  }
  else {
    callbacks.push(callback);
  }
}

Pebble.addEventListener("ready", readyCallback);
Pebble.addEventListener("showConfiguration", showConfiguration);
Pebble.addEventListener("webviewclosed", webviewclosed);
Pebble.addEventListener("appmessage", appmessage);

onReady(function(event) {
  var message = prepareConfiguration(getOptions());
  transmitConfiguration(message);
});