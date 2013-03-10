/*GOALS OF WEB PROXY:
 - act as server, get request from browser
 - act as client, make request to server
 - log responses from server to hand back to browswer 
 	(res.writeHead(chunk) from client's callback function??)*/

http = require('http');  // ask to use the http module in node.js
var entry = new Array(); //for logger, **how to set this to an array??

//callback function (takes ServerResponse obj & ServerRequest obj) gets invoked for every request to HTTP server. 
var callback = function(req, res) { //get req from browser, eventually feed res back to browswer

	var d = new Date (Date.now());
	entry[2] = d.toTimeString();
	
	//handle errors, data, and end from browser:	
	req.on('clientError', function(e) {
        console.log("Error from client-closing connection");
        req.connection.close();
    });
    
    //construct options parameter for HTTP request:
    var host_name = req.headers.host;
    var options = {
	'method': req.method, 
	'path' : req.url, 
    'headers':{'Connection':'close'}
	 };  
	
    console.log(req);
    console.log("TESTING HERE");
	//check for nonstandard port:
	var hostport = host_name.split(":");
	if (hostport.length==1) {
		options['hostname'] =  hostport[1];
		options['port'] = 80;
	} else if (hostport.length==2) {
		options['hostname'] = hostport[1];
		options['port'] = hostport[2];
	}
	//add this info to logger entry:
		entry[0]= hostport[1]+":"+hostport[2];
	//process request as client (make request to server)
   	var request = http.request(options, function(response) {
		var responsestr = "" + response.statusCode + " " + JSON.stringify(response.headers);
		console.log("Got initial response headers from server: " + responsestr);

		response.on('data', function(chunk) { 
			console.log("Got data from server: " + chunk);
		}); //handler for 'data' event invoked with response data from server

		response.on('end', function(chunk) {
			console.log("Response is complete.");
		}); //handler for 'end' event invoked after receiving entire response
     });

    req.on('data', function(chunk) {
        console.log("Received data with the request: " + chunk);
        //handle data from browser as a client to pass to it to server:
        response.write(chunk);
    }); //handler for 'data' event invoked with response data from client

    req.on('end', function() {
        console.log("Got all request information.");
        request.end();
    }); //handler for 'end' event invoked after receiving entire request
     
    request.on('error', function(e) { //error handler for request
    	console.log("Houston, we have a problem with our HTTP request: " + e);
    	request.abort();
	});
    
    req.on('clientError', function(e) {
        console.log("Error from client-closing connection");
        req.connection.close();
    });
	
	console.log("HERE."); //
        
    // send a canned response back to the client
    res.writeHead(200, {'Content-Type': 'text/plain'});
    res.end('Hello World\n');
};

/***need to figure out #1 where to initiate the logger var and #2 how to log data for every request***/
//create logger: (***MAKE SURE TO CLOSE FILES) 
/*var logger = function (filename)  {
	var logfile = require("fs").createWriteStream(filename, {'flags':'w'})
	return {
		'logit': function() {
			logfile.write(JSON.stringify(entry));
		}
	};
} (filename));*/

// create the server, pass in callback function
var server = http.createServer(callback);

// listen for new requests at the given TCP port and IP address
server.listen(8090, '127.0.0.1'); //NEED TO CHANGE THIS?
//entry[1]=argv[1] + ":" + argv[0]; //assuming listen() takes argv...

console.log('Server running at http://127.0.0.1:8090/');
