var client = require('nodejs-websocket')
//create file reader 
var fs = require('fs');
var buffer = new Buffer(0);
var data;
base64_encode('/home/shai/Pictures/test.jpg')
// buffer = Buffer.concat([buffer,data],buffer.length+data.length)
// console.log("ryu : "+data);

// function to encode file data to base64 encoded string
function base64_encode(file) {
    // read binary data
    var bitmap = fs.readFileSync(file);
    // convert binary data to base64 encoded string
    data= new Buffer(bitmap);
    // return 'ryu'
}


var conn = client.connect('ws://localhost:8001',function(str){
    console.log("connected: ");
    // conn.sendText('ryu from client')
    conn.on('text',function(str){
        console.log('from server: '+ str);
        conn.close();
    })
    conn.on('binary',function(str){
        console.log('binary : '+ str)
    })
    conn.send(data,function(inStream){
    
    })
})