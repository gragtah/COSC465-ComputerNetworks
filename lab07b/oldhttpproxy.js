/*GOALS OF WEB PROXY:
 - act as server, get request from browser
 - act as client, make request to server
 - log responses from server to hand back to browswer 
 	(res.writeHead(chunk) from client's callback function??)*/

http = require('http');  // ask to use the http module in node.js

//create logger: (***MAKE SURE TO CLOSE FILES)
/*var logger = function (filename)  { //need to first create file object?
	var logfile = require("fs").createWriteStream(filename, {'flags':'w'})
	return {
		'logit': function(whatGoesHere) { //not sure whatGoesHere
			//construct a string to log
			logfile.write(string);
		}
	};
} (filename));
*/
//callback function (takes ServerResponse obj & ServerRequest obj) gets invoked for every request to HTTP server. 
var callback = function(req, res) { //get req from browser, eventually feed res back to browswer

	//WE NEVER GET ANY DATA FROM THE BROWSER'S REQUEST.... (req.on functions)??
	
//construct options parameter for HTTP request:
    var host_name = req.headers.host;
    var options = {
	'method': req.method, 
	'path' : req.url, 
    	'headers':{'Connection':'close'}
	 };  
	
	//check for nonstandard port:
	var hostport = host_name.split(":");
	if (hostport.length==1) {
		options['hostname'] =  hostport[1];
		options['port'] = 80;
	} else if (hostport.length==2) {
		options['hostname'] = hostport[1];
		options['port'] = hostport[2];
	}

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
     
    request.on('error', function(e) { //error handler for request
    	console.log("Houston, we have a problem with our HTTP request: " + e);
    	request.abort();
	});
    
    req.on('clientError', function(e) {
        console.log("Error from client-closing connection");
        req.connection.close();
    });
    
    //WE WANT TO LOG RESPONSES TO SERVER... and then request.end()
    console.log(request); 
	request.end(); //not sure where we want this but until we call this, nothing will happen says joel
	
    // the 'data' callback gets invoked when we receive a chunk
    // of data from the client as part of their request.
    req.on('data', function(chunk) {
        console.log("Received data with the request: " + chunk);
    });

    // the 'end' callback gets invoked when the entire request
    // has been received, including data.
    req.on('end', function() {
        console.log("Got all request information.");
    });
	
	console.log("HERE."); //
        
    // send a canned response back to the client
    res.writeHead(200, {'Content-Type': 'text/plain'});
    res.end('Hello World\n');
};

// create the server, pass in callback function
var server = http.createServer(callback);

// listen for new requests at the given TCP port and IP address
server.listen(8090, '127.0.0.1');

console.log('Server running at http://127.0.0.1:8080/');
