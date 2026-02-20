# Warehouse Slotting Optimization Project

## Overview
This project implements an Object-Oriented solution for the warehouse slotting problem (product placement and routing). The architecture is designed to be modular, separating data structures, constructive heuristics, metaheuristics, and exact methods.

This document serves as a guide to the codebase, detailing the software architecture, data flow, and file responsibilities to facilitate grading and code review.

---

## 1. System Architecture
The project is organized into four distinct layers. **All files listed below are actively used** in the final compilation; there is no dead code or unused legacy files.

### A. Core Data & Orchestration
* **`Datas.hpp`**: The foundation of the project. It parses the 6 input files and stores the immutable problem instance (`Data` struct) and the mutable state (`Solution`, `Familly`, `Orders` structs).
* **`Conductor.hpp`**: The central controller ("The Brain"). It owns the `Data` and `Solution` objects. It manages the lifecycle of the program by calling initializers, preparing search structures, and triggering the selected optimization algorithm.
* **`Common.hpp`**: Contains shared utilities used globally, such as random number generation and common vector operations.

### B. Initialization & Constructive Heuristics
These files are responsible for generating a valid initial `Solution` before optimization begins.
* **`greedy.hpp`**: Implements constructive algorithms to build starting solutions.
    * **COI**: Cube Order Index heuristic.
    * **Best Insertion**: Correlation-based greedy insertion.
    * **Random**: Purely random valid placement.
* **`FamilliesTSP.hpp`**: Solves a Traveling Salesperson Problem (TSP) on product families to optimize their ordering based on correlation matrices, using the Lin-Kernighan heuristic.

### C. Optimization (Metaheuristics)
* **`LocalSearchMethods.hpp`**: The abstract base class for all local search algorithms. It manages neighborhood structures (Swap/Send operators) and efficient delta evaluation data structures (`orderAndRacks`, `aisleInfos`).
* **`GradientDescent.hpp`**: Implements First Improvement Local Search (FILS).
* **`SimulatedAnnealing.hpp`**: Implements Simulated Annealing (SAA) with the Metropolis criterion.

### D. Exact Methods
* **`Milp.hpp`**: A Mixed-Integer Linear Programming model using the Gurobi C++ API. It provides an exact mathematical resolution for benchmarking or solving smaller instances.

---

## 2. Data Flow & Execution Pipeline
The application execution is linear and strictly managed by the `Conductor` class.

1.  **Parsing**: `Data` constructor reads instance files.
2.  **Initialization Strategy**: The user (or default config) selects a strategy from `greedy.hpp` or `FamilliesTSP.hpp`:
    * *Simple:* Random or Sequential (`initSol`).
    * *Heuristic:* COI (`coiGreedy`) or Correlation-based (`GreedyBestInsert`).
    * *TSP:* Optimizes family sequence first, then fills racks.
3.  **Pre-computation**: `Conductor::computeArgs()` initializes acceleration structures (e.g., `oAr` for order-rack mapping) required for fast move evaluation.
4.  **Optimization Loop**: The `Conductor` passes the `Solution` to a solver inheriting from `LocalSearchMethods`:
    * `GradientDescent` (Hill Climbing).
    * `SimulatedAnnealing` (Global optimization).
    * `swapFamSAA` (A hybrid approach alternating between product shuffling and family swapping).
5.  **Output**: The final solution is validated and written to disk/console.

---

## 3. File Directory & Quick Reference

| File | Type | Responsibility |
| :--- | :--- | :--- |
| **`Datas.hpp`** | Structs | Data loading, Instance storage, Solution representation. |
| **`Conductor.hpp`** | Class | **Main Controller**. Connects Data to Solvers. |
| **`greedy.hpp`** | Algorithms | **Constructive Heuristics**. Generates initial solutions (COI, BestInsert). |
| **`FamilliesTSP.hpp`** | Algorithms | **Family Optimization**. Orders families via TSP. |
| **`LocalSearchMethods.hpp`** | Abstract Class | Base logic for moves (Swap/Send) and cost delta calculation. |
| **`GradientDescent.hpp`** | Algorithm | Descent (Local Search) implementation. |
| **`SimulatedAnnealing.hpp`** | Algorithm | Simulated Annealing implementation. |
| **`Milp.hpp`** | Model | Gurobi MILP implementation. |
| **`Common.hpp`** | Utils | Helper functions (Random, Math). |

## 4. Compilation and Execution

The project includes a `Makefile` to streamline the build process.

### Compilation
To compile the source code, open your terminal in the src/ folder and enter :

```bash
make
```

This command will compile all the necessary files and generate an executable named `prog`.

### Running the Program
The executable requires a single integer argument passed via the command line. This integer specifies which data instance the program should load and solve.

**Syntax:**
```bash
./prog <instance_id>
```

**Available Instances Mapping:**
* **`0`** : `toy`
* **`1`** : `big_category`
* **`2`** : `big_family`
* **`3`** : `big_market`

**Example:** To run the optimization on the `big_fam` instance, you would execute:
```bash
./prog 2
```

### Interactive Execution (Standard Input)
Once the program is launched with the chosen instance, the execution pipeline is handled interactively by the `Conductor::executeUserChoice()` method. 

The program will prompt you in the terminal via standard input (stdin) to:
1. Choose the **initialization method** (e.g., Random, COI, Best Insertion, TSP).
2. Choose the **optimization algorithm** (e.g., Simulated Annealing, Gradient Descent, MILP, etc.).

Simply follow the on-screen instructions and type the number corresponding to your choice to execute the desired pipeline.