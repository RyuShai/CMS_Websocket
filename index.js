//return nodejs-websocket 
var ws = require('nodejs-websocket')
//variable save sender connect for quick access
var senderConnection;
//variable emit stop send source data
var stopStream = false;
//variable emit webview connected
var isReceiverConnected=false;
//variable emit engince connected
var isSenderConnected =false;
//create listener websocket server
var server = ws.createServer(function(connection){
	console.log("new connect from : "+ connection.path)
	
	//check new connection path type
	//type sender: save it to senderConnection
	//type receiver : send get video image request to sender to get image then send it to receiver
	if(connection.path.includes('sender'))
	{
		//type sender		
		if(!isSenderConnected)
		isSenderConnected = true;

		senderConnection = connection;
	}
	else if(connection.path.includes('receiver'))
	{
		//type receiver
		if(!isReceiverConnected)
		isReceiverConnected=true;

		if(senderConnection != null || senderConnection != undefined)
			senderConnection.sendText('getVideo');
	}
	else
	{
		//undefine connection, close it
		console.log("stranger connection")
		connection.close(302);
	}

	//check comming message from client
	//text message
	connection.on("text",function(str){
		console.log("onText ")
		
		onTextMessage(str,connection)
	})

	//binary message
	connection.on("binary",function(inStream){
		console.log("onBinary")
		onBinaryMessage(inStream, connection)
	})

	connection.on("error",function(err){
		console.log("error: "+err)
	})

	connection.on("close",function(code,reason){
		console.log('code: '+ code + " reason: "+ reason + ' '+connection.path);
		if(code == 1001)
		{
			// console.log(senderConnection.readyState);
			// if(senderConnection.readyState== senderConnection.OPEN)
			// {
				// console.log("send stop");
				stop=true;
			// }
		}
		if(connection.path.includes('sender'))
		{
			server.connections.forEach(function(conn){
				isSenderConnected = false;
				if(con.path.includes("sender")){
					isSenderConnected=true;
				}
			})
		}
		else if(connection.path.includes('receiver'))
		{
			server.connections.forEach(function(conn){
				isReceiverConnected = false;
				if(con.path.includes("receiver")){
					isReceiverConnected=true;
				}
			})
		}
	})

	
}).listen(8224) 

server.on('error',function(err){
	console.log("error: "+ err);
})


//on text message
function onTextMessage(str, conn)
{
	console.log("Text message from : "+ conn.path)
	if(conn.path.includes('sender'))
	{
		if(stop)
		{
			console.log("send stop");
			connection.sendText('stop')
			stop = false;
		}
		else
		{
			console.log('send to reveiver');
			// from sender client
			referData2Receiver(str)
			conn.send('ok')
		}
		
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
			if(client.readyState == client.OPEN)
			{
				// console.log(data);
				client.sendText(data)	
			}
			
		}
	})
}

///////////
//create server listent text data
var jsonSever = ws.createServer(function(conn){
	//new connection connected
	console.log("new connection to 8225: "+conn.path)
	onTextDataConnect(conn);
	
	//process coming message
	conn.on("text",function(str){
		onTextDataMessage(str,conn);
	})
}).listen(8225)

function onTextDataMessage(str, conn)
{
	if(conn.path.includes('sender'))
	{
		if(isReceiverConnected)
		{
			jsonSever.connections.forEach(function(client){
				if(client.path.includes("receiver"))
				{
					//send broad cast json to receiver
					client.sendText(str);
					// console.log("to receiver: "+ str);
				}
			})
			conn.sendText('ok');
		}
		else{
			conn.send('stop')
		}	
		
	}
	conn.on("error",function(err){
		console.log("error: "+err)
	})

	conn.on("close",function(code,reason){
		console.log('code: '+ code + " reason: "+ reason + ' '+conn.path);
		// if(code == 1001)
		// {
		// 	// console.log(senderConnection.readyState);
		// 	// if(senderConnection.readyState== senderConnection.OPEN)
		// 	// {
		// 		// console.log("send stop");
		// 		stop=true;
		// 	// }
		// }
	})
}

function onTextDataConnect(conn)
{
	if(conn.path.includes('sender'))
	{

	}
	else if(conn.path.includes("receiver"))
	{
		
	}
	else
	{
		//stranger connection ==> close it
		conn.close(302)
	}
}