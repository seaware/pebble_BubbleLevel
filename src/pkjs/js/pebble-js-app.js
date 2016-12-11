
function uuid()
{
	return "af4cead0-784f-11e3-981f-0800200c9a66";
}

function baseURL()
{
	return "http://canova3.com/mobilestat/index.php/";
}

Pebble.addEventListener("ready", function() {
	console.log("Level JS ready!");
	console.log("Pebble Account Token: " + Pebble.getAccountToken());
	getUserStats();
});

function getUserStats(){
	
  var response;
  var response_parts = new Array();
  var req = new XMLHttpRequest();
	
  // build the GET request
  req.open('GET', baseURL() + "api/on/" + uuid() + "/" + Pebble.getAccountToken(), true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      // 200 - HTTP OK
      if(req.status == 200) {
		response_parts = req.responseText.split("<",1);
        response = JSON.parse(response_parts[0]);
		console.log("Web reply:" + response_parts[0]);
		sendToPebble(response);
        }
      } else {
        console.log("Web request returned error code " + req.status.toString());
      }
    }
  req.send(null);
};


function sendToPebble( response )
{
		// Send entire response to pebble
		var transactionId = Pebble.sendAppMessage( response 
			
 	    ,function(e) {
			console.log("Successfully delivered message, Id="
			  + e.data.transactionId);
		  } 
			// Pebble has an SDK bug so the following doesn't work:
			/* ,
		  function(e) {
			console.log("Unable to deliver message with transactionId="
			  + e.data.transactionId
			  + " Error is: " + e.error.message);
		  } */
		);
};

// Show config menu
Pebble.addEventListener("showConfiguration", function(e) {
	
	highlight = localStorage.getItem("highlight");
	if (typeof highlight === 'undefined') {
		highlight = "0";
	}
	orientation = localStorage.getItem("orientation");
	if (typeof orientation === 'undefined') {
		orientation = "0";
	}

	console.log("Showing configuration");
	console.log(" Highlight=" + highlight);
	console.log(" Orientation=" + orientation)
	Pebble.openURL(baseURL() + "show/config/" + uuid() + "/" + Pebble.getAccountToken() + "/" + highlight + "/" + orientation);
});

// Save config from web menu onto phone and pebble
Pebble.addEventListener("webviewclosed", function(e) {
	console.log("Configuration closed");
	console.log("  Web response=" + e.response);
	if (typeof e.response != 'undefined' && e.response != "")
	{
		var config = JSON.parse(e.response);
		//config.option1 = 1;	// Temp for testing
		//config.option2 = 2;
		localStorage.setItem("highlight", config["option1"]);
		localStorage.setItem("orientation", config["option2"]);
		sendToPebble( config );
	}
});