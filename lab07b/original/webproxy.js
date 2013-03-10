/* 
 * Lab 7b
 * Gaurav Ragtah, Caroline Cragin, AJ Ganik
 */

//Usage: node httpproxy.js <filename>

var http = require('http');  // ask to use the http module in node.js


// Logging functionality. argv[2] is log filename.
var logfile = (function(filename){
	filehandler = require("fs").createWriteStream(filename, {'flags':'w'});

	return {'logit': function(clientIP, clientPort, host, port, requesttime, finishtime, methodURIhttpver, rescode, size, latency){
		var throughput = 0;		
                if (latency > 0) { throughput = (size*8) / latency;}
			
                var out = clientIP + ":" + clientPort + " " + host + ":" + port + " " + "[" + requesttime + "] [" + finishtime + "] " + methodURIhttpver + " " + rescode + " " + size + " " + latency + " " + throughput.toFixed(2) + "\n" ;
                filehandler.write(out);
            }
	}
}(process.argv[2]));



var responsecode = 0,
size = 0;

var callback = function(req, res) {
	var start = Date.now();
	
        var host_name = req.headers.host || req.headers.Host;
        var options = {
            'method': req.method, 
            'path' : req.url, 
            'headers': req.headers
        };  
	
	//check for nonstandard port
	var hostport = host_name.split(":");
	if (hostport.length==1) {
		options['hostname'] =  hostport[0];
		options['port'] = 80;
	} else if (hostport.length==2) {
		options['hostname'] = hostport[0];
		options['port'] = hostport[1];
	}


        req.on('clientError', function(e) {
            console.log("Error from client-closing connection");
            req.connection.close();
        });

        //Handle data from browser as a client to pass to it to server:
	req.on('data', function(chunk) {
		console.log("Received data with the request: " + chunk);
		request.write(chunk);
	});

        //Handler for 'end' event invoked after receiving entire request
	req.on('end', function() {
		console.log("Got all request information.");	
		request.end();
        });

// Client behavior START

	var request = http.request(options, function(response) {
		var responsestr = "" + response.statusCode + " " + JSON.stringify(response.headers);
		console.log("Got initial response headers from server: " + responsestr);
		var responsecode = response.statusCode;

		//Extract headers from response and write to res
    	        res.writeHead(response.statusCode, response.headers);

		//write the chunk to the res		
		response.on('data', function(chunk) {
			if (chunk) { size += chunk.length;}
			res.write(chunk);
			console.log("Got data from server: " + chunk); 
		});

		// Handler for the 'end' event 
		response.on('end', function(chunk) {
			//logging stuff
			var methodURIhttpver = "\"" + options.method + " http://" + options.hostname + " HTTP/1.1\"",
			clientIP = req.connection.remoteAddress,
			clientPort = req.connection.remotePort;
			res.end(chunk); 
			
                        var end = Date.now(); 
			var latency = (end - start)/1000;

			logfile.logit(clientIP, clientPort, options.hostname, options.port,new Date(start),new Date(end), methodURIhttpver, responsecode, size, latency);
			console.log("Response is complete.");
			size = 0;
		});
	});


        request.on('error', function(e) { //error handler for request
            console.log("Houston, we have a problem with our HTTP request: " + e);
            request.abort();
            });

// Client behavior END

};


// create the server, giving the main callback function
var server = http.createServer(callback);

// listen for new requests at the given TCP port and IP address
server.listen(8090, '127.0.0.1');

console.log('Server running at http://127.0.0.1:8090/');
