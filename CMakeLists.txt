cmake_minimum_required(VERSION 3.0.0)
project(Athena VERSION 1.0.0)

add_executable(Athena 
                src/Athena.cpp
                src/Athena.h
                src/Bitboard.h
                src/Board.cpp
                src/Board.h
                src/Bitboard.cpp
                src/ChessPosition.h
                src/ChessGame.cpp
                src/ChessGame.h
                src/Constants.h
                src/DataTypes.h
                src/Eval.cpp
                src/Eval.h
                src/main.cpp
                src/MoveData.h
                src/MoveGeneration.h
                src/MoveGeneration.cpp
                src/Outcomes.cpp
                src/Outcomes.h
                src/SquarePieceTables.h
                src/TranspositionHashEntry.h
                src/utils.h
                src/utils.cpp
                src/UCI.h
                src/UCI.cpp
                src/ZobristKeyGenerator.h
                src/ZobristKeyGenerator.cpp
)