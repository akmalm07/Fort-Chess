const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });

let queue = []; // queue for players waiting for a match

wss.on('connection', function connection(ws) {
    console.log('Client connected');

    // Add client to the queue
    queue.push(ws);
    console.log(`Queue size: ${queue.length}`);

    // If we have 2 players, start a game
    if (queue.length >= 2) {
        const player1 = queue.shift();
        const player2 = queue.shift();

        // Send color assignments
        player1.send('WHITE');
        player2.send('BLACK');

        console.log('Match started: WHITE vs BLACK');

        // Optionally, set up message forwarding between them
        const forward = (from, to) => {
            from.on('message', (msg) => {
                // Forward only if both sockets are still open
                if (to.readyState === WebSocket.OPEN) {
                    to.send(msg.toString());
                }
            });
        };

        forward(player1, player2);
        forward(player2, player1);
    }

    ws.on('close', () => {
        console.log('Client disconnected');
        // Remove from queue if they disconnect before a match
        queue = queue.filter(c => c !== ws);
    });

    ws.on('message', (message) => {
        console.log('Received from client:', message.toString());
        // Could also process server commands here if needed
    });
});

console.log('WebSocket server running on ws://localhost:8080');
