/**
 * @author Gustavo Y. Maruyama <gustavo.maruyama@ifms.edu.br>
 */
 
const aedes = require('aedes')();

// Servidor TCP - para atender clientes que possam utilizar o protocolo MQTT sobre a camada TCP diretamente
const tcpServer = require('net').createServer(aedes.handle);
const tcpPort = 1883;

// Servidor HTTP e Websocket - para atender clientes que possam utilizar o protocolo MQTT apenas sobre websocket - ex. navegador web
// MQTT over Websocket
const httpServer = require('http').createServer();
const Websocket = require('ws');
const wsServer = new Websocket.Server({ server: httpServer });
const wsPort = 8080;

aedes.on('client', function (client){
	console.log("Novo cliente conectado");
});

aedes.authorizePublish = function(client, packet, callback){
	console.log("Publish recebido pelo broker");
	console.log(packet);
	const payload = packet.payload.toString();
	console.log("Mensagem: ",payload);
	callback(null, packet);
}

aedes.authorizeSubscribe = function(client, packet, callback){
	console.log("Cliente realizou um subscribe");
	console.log(packet);
	callback(null,packet);
}

wsServer.on('connection', function connection (ws) {
	const duplex = Websocket.createWebSocketStream(ws)
	aedes.handle(duplex)
});

tcpServer.listen(tcpPort, function(){
	console.log("Broker MQTT Aedes escutando na porta ", tcpPort);
});

httpServer.listen(wsPort, function () {
	console.log('Servidor HTTP e Websocket escutando na porta', wsPort);
});