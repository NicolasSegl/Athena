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

## Building Athena
>mkdir build<br/>
>cd build<br/>
>cmake ..<br/>
>make<br/>

When running the newly made executable, ensure that /books/ is in Athena's current working directory, otherwise she won't be able to use the opening book. 
