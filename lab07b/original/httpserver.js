var http = require('http');  // ask to use the http module in node.js

// here's our callback function: this will get invoked for every
// request to our HTTP server.  the callback takes a ServerRequest object
// and ServerResponse object as parameters (see the node documentation
// for details).  We pass this function into the createServer method
// below.
var callback = function(req, res) {

    console.log("testt" + req.connection.remoteAddress);
    // set up a callback for when there's an error on the client
    // connection.
    req.on('clientError', function(e) {
        console.log("Error from client-closing connection");
        req.connection.close();
    });

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

    // send a canned response back to the client
    res.writeHead(200, {'Content-Type': 'text/plain'});
    res.end('Hello World\n');
};

// create the server, giving the main callback function
var server = http.createServer(callback);

// listen for new requests at the given TCP port and IP address
server.listen(8080, '127.0.0.1');

console.log('Server running at http://127.0.0.1:8080/');
