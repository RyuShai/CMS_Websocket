//return nodejs-websocket 
var ws = require('nodejs-websocket')
//variable save sender connect for quick access
var senderConnection;
//create listener websocket server
var server = ws.createServer(function(connection){
	console.log("new connect from : "+ connection.path)
	//check comming message from client
	//text message
	connection.on("text",function(str){
		console.log("onText "+str)
		// onTextMessage(str,connection)
	})

	//binary message
	connection.on("binary",function(inStream){
		console.log("onBinary")
		onBinaryMessage(inStream, connection)
	})
	//check new connection path type
	//type sender: save it to senderConnection
	//type receiver : send get video image request to sender to get image then send it to receiver
	if(connection.path.includes('sender'))
	{
		//type sender
		console.log("damn")
		connection.send("getVideo");
		senderConnection = connection;
	}
	else if(connection.path.includes('receiver'))
	{
		//type receiver
		senderConnection.sendText('getVideo');
	}
	else
	{
		//undefine connection, close it
		console.log("stranger connection")
		connection.close(302);
	}

	

	
}).listen(8224) 

//on text message
function onTextMessage(str, conn)
{
	console.log("Text message from : "+ conn.path)
	if(conn.path.includes('sender'))
	{
		console.log(str);
		//from sender client
		referData2Receiver(str)
	}
	else if( conn.path.includes("receiver"))
	{
		//from receiver client

	}
	else{
		//from stranger client, close it
		conn.close()
	}
}
//on binary message
function onBinaryMessage(inStream, conn)
{
	console.log("Binary message from : "+ conn.path)
	if(conn.path.includes('sender'))
	{

		//from sender client
		//refer to receiver

	}
	else if( conn.path.includes("receiver"))
	{
		//from receiver client

	}
	else{
		//from stranger client, close it
		conn.close()
	}
}

//broadcast to refer data to receiver
function referData2Receiver(data)
{
	server.connections.forEach(function(client){
		//send data to receiver client
		if(client.path.includes('receiver'))
		{
			client.send(data,function(err){
				console.log('respond from send: '+err)
			})
		}
	})
}