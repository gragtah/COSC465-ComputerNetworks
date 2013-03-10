 function bar() {

   console.log("bar at global scope");

 }

function foo() {

  bar();

  function bar() {

    console.log("How did I get here?");

    console.log("something is: " + something);

    console.log("nothing is: " + nothing);

  }

  var something = 10;

  console.log(bar);

}

foo();
