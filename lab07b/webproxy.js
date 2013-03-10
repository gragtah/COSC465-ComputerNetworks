/*+++++++++++++++++++++++Lab 7b -Nell Lapres, Slava Fedorchuk and Roberto Segebre+++++++++++++++++++++++*/

var http = require('http');  // ask to use the http module in node.js


/*Logging function:
 * uses argv[2] as filename and writes an entry per request sent by browser.
 */
var logfile = (function(filename){
	var logfilename = filename,
	filehandle = require("fs").createWriteStream(logfilename, {'flags':'w'});

	return {'logit': function(clientIP, clientPort, host, port, request_time, finish_time, method, HRes,size, elapsed){
		var tranlat	= 0;		
			if (elapsed > 0) {
				tranlat = (size*8) / elapsed;
			}
			
		    var string = clientIP + ":" + clientPort + " " + host + ":" + port + " " + "[" + request_time + "] [" + finish_time + "] " + method + " " + HRes + " " + size + " " + elapsed + " " + tranlat.toFixed(2) + "\n" ;
		    filehandle.write(string);
		    }
	}
}(process.argv[2]));



/*Log Variables:
 * gGts reset to 0 at the req end.
 */
var HTTPResp = 0,
size =0;
 
/* Callback:
 *  Gets invoked on every request to the Http server
 */
var callback = function(req, res) {
	var start = Date.now();
	
	//Obtaining information from req received
	var requested_url = req.url,
	host_name = req.headers['host'] || req.headers['Host'],
	method = req.method,
	port = 80,
	temp = host_name.split(':');
	
	//Extracting and setting port if any otherwise default to port 80/
	if (temp.length > 1) {
		port = temp[1];
		host_name = temp[0];
	}

	//Populate options from elements extracted above.
	var options = {
	    'hostname':host_name,
	    'port':port,
	    'method':method,
	    'path':requested_url,
	    'headers': req.headers
	};

	//Set an error handler for the closing-connection w/ client
	req.on('clientError', function(e) {
		console.log("Error from client-closing connection" + "\n");
        req.connection.close();
	});

	// Gets invoked when we receive a chunk
	// of data from the client as part of their request.
	req.on('data', function(chunk) {
		console.log("Received data with the request: " + chunk + "\n");
		request.write(chunk);
	});

	//'end' callback gets invoked request has been received, including data.
	req.on('end', function() {
		console.log("Got all request information.");	
		request.end();
    });

/*++++++++++++++++++START HTTP Client request to webserver++++++++++++++++++++++*/
	var request = http.request(options, function(response) {
		var responsestr = "" + response.statusCode + " " + JSON.stringify(response.headers);
		console.log("Got initial response headers from server: " + responsestr +"\n" );
		var HTTPResp = response.statusCode;

		//extract all the headers from the response and write to res to browser
    	res.writeHead(response.statusCode, response.headers);

		//write the chunk to the server response to the browser			
		response.on('data', function(chunk) {
			//Check chunk size if > 0 update			
			if (chunk)
				size += chunk.length;
			//Set HTTPresp to log it later on
			res.write(chunk);
			console.log("Got data from server: " + chunk + "\n");        
		});

		// Set a handler for the 'end event' 
		// Takes care of logging
		response.on('end', function(chunk) {
			//Set variables for logging
			method = options.method + " http://" + options.hostname + " HTTP/1.1",
			clientIP = req.connection.remoteAddress,
			clientPort = req.connection.remotePort;
			res.end(chunk);//Response to browser -> end communication
			var end = Date.now(); //get end time of request
			elapsed = (end - start)/1000;//transform to seconds
			logfile.logit(clientIP, clientPort, options.hostname, options.port,new Date( start),new Date( end), method, HTTPResp, size, elapsed);
			size = 0;
			console.log("Response is complete.");
		});
	});


	// set an error handler for the request
	request.on('error', function(e) { 
		console.log("Honduras, we have a problem with our HTTP request: " + e +"\n");
		request.abort();
	});
/*++++++++++++++++++END HTTP Client request to server++++++++++++++++++++++*/
};

// create the server, giving the main callback function
var server = http.createServer(callback);

/* listen for new requests at the given TCP port and IP address
Uncomment to make ip as a parameter
ip_addr = process.argv[3];
port = process.argv[4];
*/

var ip_addr = '127.0.0.1';
var port = 8081;


server.listen(port, ip_addr);
console.log('Server running at http://' + ip_addr + '/');
