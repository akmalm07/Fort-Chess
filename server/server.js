import express from 'express';
import { WebSocketServer, WebSocket } from 'ws';
import path from 'path';
import admin from 'firebase-admin';

admin.initializeApp({
    credential: admin.credential.applicationDefault(), // Uses Cloud Run's service account
});

const db = admin.firestore();

const app = express();
const PORT = process.env.PORT || 8080;

// Serve static files
app.use(express.static(path.join(process.cwd(), 'build')));
app.get('/', (req, res) => {
    res.sendFile(path.join(process.cwd(), 'build', 'index.html'));
});

const server = app.listen(PORT, () => {
   console.log(`Server listening on port ${PORT}`);
});

const wss = new WebSocketServer({ server });

// Maps Firestore playerId to actual ws connection
const connections = new Map();

const STD_LOSE = 7;

// Helper to forward messages
const forward = (from, to) => {
    from.on('message', (msg, isBinary) => {
        const buf = Buffer.from(msg);

        if (buf[0] === STD_LOSE) {
            console.log(`Player ${buf[1]} has lost the game.`);

            if (to.readyState === WebSocket.OPEN) {
                to.send(buf, { binary: true });
            }
            if (from.readyState === WebSocket.OPEN) {
                from.send(buf, { binary: true });
            }

            to.close(1000, "Game ended");
            from.close(1000, "Game ended");
            return;
        }

        // Normal forwarding
        if (to.readyState === WebSocket.OPEN) {
            to.send(buf, { binary: isBinary });
        }
        if (from.readyState === WebSocket.OPEN) {
            from.send(buf, { binary: isBinary });
        }
    });
};


// Match as many pairs as possible
  async function tryMatchPlayers() {
    const snapshot = await db.collection('queue')
      .where('status', '==', 'waiting')
      .orderBy('createdAt')
      .get();

    const waiting = snapshot.docs;

    while (waiting.length >= 2) {
      const doc1 = waiting.shift();
      const doc2 = waiting.shift();

      const player1 = connections.get(doc1.id);
      const player2 = connections.get(doc2.id);

      if (player1 && player2) {
        player1.send(new Uint8Array([0, 0])); // WHITE
        player2.send(new Uint8Array([0, 1])); // BLACK
        console.log(`Match started: ${doc1.id} (WHITE) vs ${doc2.id} (BLACK)`);

        await Promise.all([
          db.collection('queue').doc(doc1.id).update({ status: 'matched' }),
          db.collection('queue').doc(doc2.id).update({ status: 'matched' }),
        ]);

        forward(player1, player2);
        forward(player2, player1);
      }
    }
  }

wss.on('connection', async (ws) => {
    console.log('Client connected');

    // Add to Firestore queue
    const playerRef = await db.collection('queue').add({
      status: 'waiting',
      createdAt: new Date(),
    });
    const playerId = playerRef.id;

    connections.set(playerId, ws);
    console.log(`Player added to Firestore queue: ${playerId}`);

    // Try to match players every time a new one joins
    await tryMatchPlayers();

    // Handle disconnect
    ws.on('close', async () => {
      console.log(`Client disconnected: ${playerId}`);
      connections.delete(playerId);
      await db.collection('queue').doc(playerId).delete();
    });

    ws.on('message', (message) => {
      const buf = Buffer.from(message);
      console.log('Received:', buf);
    });
});
