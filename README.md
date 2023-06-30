# Athena
Athena is a UCI compatible chess engine.

## Features
* Negamax
* Alpha Beta Pruning
* Quiescence Move Search
* Killer Move Ordering Heuristic
* MVV_LVA Ordering Heuristic
* History Move Ordering Heuristic
* Null Move Pruning
* Delta Pruning
* Principal Variation Search
* Transposition Table
* Aspriation Windows
* Extensions
    * Check Extensions
    * Recapture Extensions
    * Pawn Promotion Extensions
* Iterative Deepening
* Transposition Table
* Static Exchange Evaluation Pruning (SEE)
* Piece Square Tables
* Opening Book

## Building Athena
>mkdir build<br/>
>cd build<br/>
>cmake ..<br/>
>make<br/>

When running the newly made executable, ensure that /books/ is in Athena's current working directory, otherwise she won't be able to use the opening book. 