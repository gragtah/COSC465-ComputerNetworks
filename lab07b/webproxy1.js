var http = require('http');  // ask to use the http module in node.js

var logfile = (function(filename){
	var logfilename = filename,
	filehandle = require("fs").createWriteStream(logfilename, {'flags':'w'});

	return {
		'logit': function(clientIP, clientPort, host, port, request_time, finish_time, method, HRes,size, elapsed){
			var tranlat = (size*8) / elapsed;
		    var string = clientIP + " : " + clientPort + " " + host + " : " + port + " " + "[" + request_time + "] [" + finish_time + "] " + method + " " + HRes + " " + size + " " + elapsed + " " + tranlat+ "\n" ;
		    filehandle.write(string);
		    }
	}
}(process.argv[2]));


// Callback - Gets invoked on every request to the Http server
var callback = function(req, res) {
	
	var request_time = Date.now(),
	start = Date(Date.now());
	

	//Obtaining information from req received
	var requested_url = req.url,
	host_name = req.headers['host'] || req.headers['Host'],
	method = req.method,
	data = '',
	port = 80,
	headers = JSON.stringify(req.headers),
	temp = host_name.split(':');
	
	//Extracting and setting port if any otherwise default to port 80/
	if (temp.length > 1) {
		port = temp[1];
		host_name = temp[0];
	}
	
	var options = {
	    'hostname':host_name,
	    'port':port,
	    'method':method,
	    'path':requested_url,
	    'headers':{'Connection':'close'}
	};
	var HTTPResp = 0,
	size = 0;
	var request = http.request(options, function(response) {
		var responsestr = "" + response.statusCode + " " + JSON.stringify(response.headers);
		console.log("Got initial response headers from server: " + responsestr);
		HTTPResp = response.statusCode;
		//extract all the headers from the response
    	res.writeHead(response.statusCode, response.headers);

		//write the chunk to the server response to the browser			
		response.on('data', function(chunk) {
			size += chunk.length;
			console.log("asdfg" + typeof(chunk));			
			res.write(chunk);
			console.log("Got data from server: " + chunk);        
		});

		// set a handler for the 'end event'
		response.on('end', function(chunk) {
			//size += chunk.length;    
			res.end(chunk);
			console.log("Response is complete.");
		});
	});

	// set an error handler for the request
	request.on('error', function(e) { 
		console.log("Houston, we have a problem with our HTTP request: " + e);
		request.abort();
	});
	
	// set an error handler for the closing-connection w/ client
    req.on('clientError', function(e) {
        console.log("Error from client-closing connection");
        req.connection.close();
    });

    // the 'data' callback gets invoked when we receive a chunk
    // of data from the client as part of their request.
    req.on('data', function(chunk) {
        console.log("Received data with the request: " + chunk);
	request.write(chunk);
    });

    //'end' callback gets invoked request has been received, including data.
    req.on('end', function() {
        console.log("Got all request information.");
		//log vars
		var finish_time = Date.now(),
		end = Date(Date.now()),
	  	elapsed = (finish_time - request_time)/1000,
		method = options.method + " http://" + options.hostname + " HTTP/1.1",
		clientIP = req.connection.remoteAddress,
		clientPort = req.connection.remotePort;
		console.log("Req:" + request_time);
		console.log("FIN: " + finish_time);
		console.log("Size: " + size);

	
		logfile.logit(clientIP, clientPort, options.hostname, options.port, start, end, method, HTTPResp, size, elapsed);
		request.end();
    });
};

// create the server, giving the main callback function
var server = http.createServer(callback);

// listen for new requests at the given TCP port and IP address
server.listen(8080, '127.0.0.1');

console.log('Server running at http://127.0.0.1:8080/');
