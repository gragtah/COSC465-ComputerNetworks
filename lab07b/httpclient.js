var http = require('http');  // ask to use the http module in node.js

// the "options" object for our request.  the various
// request details are just hard-coded for now...
var options = {
    'hostname':'cs.colgate.edu',
    'port':80,
    'method':'GET',
    'path':'/cs',
    'headers':{'Connection':'close'}
};

// create the request, supplying an anonymous callback function to 
// handle the response coming back from the server.
var request = http.request(options, function(response) {
	//request = http.ClientRequest object
	//response = http.ClientResponse object
	//options = request client is making
	//callback is function(response) = response side of that
    var responsestr = "" + response.statusCode + " " + JSON.stringify(response.headers);
    console.log("Got initial response headers from server: " + responsestr);

    // set a handler for the 'data' event: this gets invoked when we get
    // response data from the server
    response.on('data', function(chunk) {
        console.log("Got data from server: " + chunk);        
    });

    // set a handler for the 'end event: this gets invoked when we've
    // received the entire response from the server.
    response.on('end', function(chunk) {
        console.log("Response is complete.");
    });
});

// set an error handler for the request in case anything goes wrong
request.on('error', function(e) { 
    console.log("Houston, we have a problem with our HTTP request: " + e);
    request.abort();
});

// calling 'end' tells the request that we're ready to do (we have no
// more data to send as part of the request).  at this point, the
// connection is made and the wheels are set in motion.  **Until we
// call the end method nothing happens!**  
request.end();

//req and res also exist here
