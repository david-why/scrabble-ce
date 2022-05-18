# scrabble
Now you can play [Scrabble](https://en.wikipedia.org/wiki/Scrabble) with your friends on the TI-84+ CE!

## Basic game controls
- `up/down/left/right`: Move cursor
- `A-Z`: Place letter tile
- `[space]`: Place the `?` tile (the blank tile in the original game)
- `del`: Delete this tile (must be a tile you placed in this round)
- `mode`: End your turn
  - `1-7`: Select tiles to exchange if prompted
- `enter`: Calculate the score of a word
  - `up/down/left/right`: Select word direction (up=down and left=right)
  - `+`: Add to your score
  - `-`: Subtract from your score

## Multi-calc mode usage
This mode allows you to play (pseudo-)multiplayer, with more than one calculators! To use this:
1. Send both `SCRABBLE.8xp` AND `ASCRABLE.8xp` to everyone's calc.
2. On the first player's calc, run `Asm(prgmSCRABBLE`. When asked for "Multi-calc mode?" press `enter`.
3. Finish your turn. The program should quit by itself.
4. Connect your calc and the next player's calc with a cable.
5. Let the next player run `prgmASCRABLE`. (It is a TI-BASIC program, so you don't need the `Asm(`.)
6. This should start the game on their calc. After they finish, make sure they DO NOT type anything for the "CONTINUE?" prompt - that's for later.
7. Let the next player run `prgmASCRABLE`, and so on until everyone made their first turn.
8. Now, you (the first player) should connect your calc with the last palyer and run `prgmASCRABLE`.
9. After you finish, connect with the next player and let them type anything but `0` in the "CONTINUE?" prompt.
10. When the game is over or you want to stop, simply have everyone type `0` at the "CONTINUE?" prompt (or terminate the program with `ON`).
