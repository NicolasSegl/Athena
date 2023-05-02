# Athena
Athena is a UCI compatible chess engine.

## Features
* UCI compatability
* Negamax with alpha beta pruning
* Quiescence move search
* Piece square tables
* Opening book
* Killer move ordering heuristic
* MVV_LVVA ordering heuristic
* Null-move pruning
* Static search evaluation
* Delta pruning
* Principal Variation Search
* History Heuristic

<<<<<<< HEAD
## Running Athena
Building:
=======
## Building Athena
>>>>>>> d19ab16d756cfa0b98a6ef64123f7a776c24f71a
>mkdir build<br/>
>cd build<br/>S
>cmake ..<br/>
>make<br/>

When running the newly made executable, ensure that /books/ is in Athena's current working directory, otherwise she won't be able to use the opening book. 
