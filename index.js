var ws = require("nodejs-websocket")
var fs = require('fs')
// Scream server example: "hi" -> "HI!!!"
var data;
var server = ws.createServer(function (conn) {

	console.log("New connection")
	conn.on("text", function (str) {
		console.log("Received ")
		data+=str;
		conn.sendText("hehe")
		base64_decode(data,'received.jpg');
		// fs.writeFile("receivedText.jpg",data);
	})
	conn.on("close", function (code, reason) {
		console.log("Connection closed")
	})

	conn.on("binary",function(inStream){
		console.log('receive binary: ')
		var data = new Buffer(0);
		inStream.on('readable',function(){
			var newData = inStream.read();
			if(newData){
				data = Buffer.concat([data,newData],data.length+newData.length);
			}
		})

		inStream.on('end',function(){
			console.log('end of tranfer binary')
			base64_decode(data,'received.jpg');
		})
	})
}).listen(8001)

function processMyData(data){
	fs.writeFile("received.jpg",data,function(err){
		console.log("error: "+err);
	})
}

// function to create file from base64 encoded string
function base64_decode(base64str, file) {
    // create buffer object from base64 encoded string, it is important to tell the constructor that the string is base64 encoded
    var bitmap = new Buffer(base64str);
    // write buffer to file
    fs.writeFileSync(file, base64str);
    console.log('******** File created from base64 encoded string ********');
}