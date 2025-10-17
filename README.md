# 🏰 Fort Chess

Fort Chess is a chaotic, real-time twist on the classic game of chess — blending the strategic thinking of chess with the fast-paced, unpredictable nature of Fortnite.
Built in C++ with Raylib, the game lets everyone move pieces whenever they want, place walls by double-clicking pawns, and battle against cooldowns for control of the board.

## 🎮 Features

Real-time gameplay — no turns, anyone can move anytime.

Cooldown system — each piece has a 3-second cooldown after moving.

Wall-building mechanic — double-click pawns to place walls on the board.

Cloud-hosted multiplayer — all players share the same real-time board state.

Built with C++ and Raylib — lightweight and fast rendering for smooth play.

## 🧠 Inspiration

I wanted to mix the strategy of chess with the action and chaos of Fortnite.
The idea was: what if chess didn’t have turns, and you could literally build walls to defend your king? That idea became Fort Chess.

## ⚙️ How It Works

The game is fully real-time — meaning any player can move any piece whenever they want.
A backend running in the cloud handles all synchronization, ensuring every move is reflected for all players instantly.

Each move triggers a 3-second cooldown on that piece to prevent spam.

## 🕹️ Game Rules & Click Mechanics
🔴 Selecting & Moving Pieces

When you click a piece, it turns red — this means it’s selected.

Click any other square to move the selected piece there.

If you click the same piece again, it becomes unselected (unless it’s a pawn — see below).

### 🔵 Cooldown

If a piece is blue, it’s in cooldown — you can’t move it yet.

Cooldown lasts 3 seconds after each move.

###  Wall Building (Pawns Only)

Double-click a pawn to enter wall-building mode.

The next square you click will determine where the wall appears.

If you click the pawn three times, it unselects and cancels wall placement.

No piece can move through walls, except rooks, who cna break walls

### 🏗️ How We Built It

Language: C++

Graphics: Raylib

Networking: Custom cloud backend for real-time synchronization

Logic: Custom event loop managing selection, cooldowns, and wall placement

## 💪 Challenges

Synchronizing multiple players in real time.

Managing conflicting moves when two players act at the same time.

Implementing cooldown logic that feels fair and responsive.

Making wall placement intuitive and bug-free.

Deploying and maintaining a stable cloud environment.

## 🏆 Accomplishments

Created a unique hybrid of chess and action gameplay.

Deployed a working real-time multiplayer system.

Learned a lot about networked game state, Raylib rendering, and C++ optimization.

## 📘 What We Learned

Handling concurrency and fairness in real-time systems.

Structuring scalable multiplayer logic.

Deploying C++ applications to the cloud efficiently.

## 🚀 What’s Next for Fort Chess

Add more piece abilities and power-ups.

Implement matchmaking and private rooms.

Improve graphics, sound, and animations.

Release a web-playable version for easier access.

## 🧩 Controls Summary
Action	Description
🖱️ Click piece (red)	Select piece
🖱️ Click same piece again	Unselect piece (except pawn)
🖱️ Click empty square	Move selected piece
🔵 Blue piece	Piece in 3s cooldown
🖱️ Double-click pawn	Enter wall placement mode
🖱️ Click any square after double-click	Place wall
🖱️ Triple-click pawn	Cancel wall placement
💬 Final Thoughts


Fort Chess turns the quiet strategy of chess into a battlefield of quick thinking and reflexes.
Every second counts — build walls, plan ahead, and outsmart your opponents in real time!
