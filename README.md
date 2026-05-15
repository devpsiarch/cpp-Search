# Search and Optimization Library

A modular C++ framework for solving complex search problems, ranging from constraint satisfaction to evolutionary and stochastic adversarial games.

## Core Framework

The library is built on a set of abstract header files that define the interface for different search paradigms:

-   **`search.hpp`**: The base interface for state-space search problems.
    
-   **`csp.hpp`**: Framework for Constraint Satisfaction Problems, managing variables, domains, and constraints.
    
-   **`adversarial_search.hpp`**: Deterministic game-playing logic (e.g., Minimax, Alpha-Beta pruning).
    
-   **`sa_search.hpp`**: **Stochastic Adversarial Search** engine for games involving elements of chance or uncertainty.
    
-   **`local_search.hpp`**: Trajectory-based optimization interfaces (e.g., Hill Climbing).
    
-   **`genetic_search.hpp`**: Population-based evolutionary algorithms.
    

## Implementations

### Constraint Satisfaction (CSP)

-   **`simple_csp.cpp`**: A basic implementation of the CSP interface.
    
-   **`interleaving_ac3.cpp`**: An optimized solver utilizing the AC-3 algorithm to maintain arc consistency during search.
    

### N-Queens Puzzle

-   **`local_search_nqueens.cpp`**: Solves the N-Queens problem using iterative improvement (Hill Climbing).
    
-   **`genetic_nqueens.cpp`**: Uses a population-based approach with crossover and mutation to find valid board configurations.
    

### Adversarial & Stochastic Games

-   **`advers.cpp`**: Implementation of standard competitive search.
    
-   **`luck_advers.cpp`**: Implementation of games incorporating "luck" or random events using the stochastic adversarial framework.
    

### Navigation & Utilities

-   **`maze.cpp`**: Pathfinding algorithms applied to grid-based navigation.
    
-   **`viz.cpp`**: A utility suite for visualizing search progress and state transitions.
    

----------

## Technical Features

-   **Generic Design**: The solvers are decoupled from the specific problem definitions, allowing any new problem to be solved by multiple algorithms without code changes.
    
-   **Pruning**: Includes advanced techniques like Alpha-Beta pruning in adversarial search and AC-3 in CSPs to reduce the search space.
    
-   **Stochasticity**: Dedicated support for chance nodes in game trees via Expectiminimax or similar stochastic logic.
    

----------

## TODO

-   **Particle Swarm Optimization (PSO)**: Implement a swarm-based meta-heuristic to model collective movement toward global optima.
    
-   **Ant Colony Optimization (ACO)**: Implement pheromone-based pathfinding for discrete optimization tasks.
    
-   **Hybrid Solvers**: Combine AC-3 filtering with local search to handle heavily constrained optimization problems.

