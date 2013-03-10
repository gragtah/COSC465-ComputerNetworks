 var myobj = (function() {

     var count = 0;

     return {

click: function() {

if (arguments.length === 1) {

count += arguments[0];

} else {

count += 1;

}

},

getClicks: function() {

             return count;

           }

};

}());

console.log(myobj);

myobj.click();

console.log(myobj.getClicks());

myobj.click(5);

console.log(myobj.getClicks());
