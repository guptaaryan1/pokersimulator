# Parallel Poker Preflop Simulator

Concurrent Monte Carlo simulation for Texas Hold'em preflop odds, written in C++.

## Features
- Heads up preflop equity
- Multi-threaded Monte Carlo simulations using `std::thread`.

## Build
Compile with any C++11 compatible compiler (needs pthread on Linux/Mac):
```bash
g++ -O3 -pthread pokersimulator.cpp -o pokersimulator
```
*(Or just open the `.sln` in Visual Studio and build in Release mode).*

## Run
```bash
./pokersimulator
```
Follow the prompts to enter hole cards (e.g., `Ah Kh` vs `Qs Qd`), the number of simulations, and your desired thread count.