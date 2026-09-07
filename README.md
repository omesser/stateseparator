# Quantum State Separator

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](LICENSE)

**Check whether a multi-qudit density matrix is separable or entangled** — and, when it is separable (or close), see a nearest separable approximation and its decomposition into product states.

The tool runs a [Peres–Horodecki](https://en.wikipedia.org/wiki/Peres%E2%80%93Horodecki_criterion) (PPT) partial-transpose test, then a probabilistic numerical search for the nearest separable state based on *Geometrical aspects of entanglement*, Physical Review A **74**, 012313 (2006), by Jon Magne Leinaas, Jan Myrheim, and Eirik Ovrum.

## Contents

- [Try it](#try-it)
- [Quick start](#quick-start)
- [Optional parameters](#optional-parameters)
- [Main algorithm](#main-algorithm)
- [Accuracy](#accuracy)
- [Time complexity](#time-complexity)
- [Want to help?](#want-to-help)
- [History and credits](#history-and-credits)
- [License](#license)
- [Appendix: Matrix-element format](#appendix-matrix-element-format)

## Try it

[Open the State Separator](https://stateseparator.onrender.com)

Paste a density matrix, enter the qudit dimensions, hit **Separate**.

## Quick start

1. Paste the density matrix into the main window (whitespace-separated elements; newline = new row).
2. Enter qudit number and dimensions, e.g. `2 2` for two qubits, or `2 3` for qubit+qutrit.
   The product of the dimensions must equal the matrix order.
3. Optionally tweak target distance, minimum weight, target number of states, output precision, or accuracy boost.
4. Click **Separate**.

### How to read the result

| Verdict | Meaning |
| --- | --- |
| `entangled (Peres Test)` | Failed the Peres–Horodecki PPT test (necessary for separability; also sufficient for `2×2` and `2×3`). |
| `separable` | Distance to the nearest separable approximant reached the target (default `0.5E-13`). |
| `might be entangled` | Distance &lt; `0.0005` but above the target. |
| `most likely entangled` | Larger residual distance. |

Full numeric details (nearest separable state, product-state weights, distance) appear below the verdict. You can hit **Separate** again without clearing the window; previous output is ignored as input.

Matrix elements may look like `+2.3-i0.2`, `5i`, `-0.2i`, `0.6i-3.1`. See [Appendix: Matrix-element format](#appendix-matrix-element-format) for the full grammar.

The matrix is checked (to about 3 decimal digits) for Hermitian, positive semidefinite, and unit trace. Warnings do not stop the run, but they affect how meaningful the output is — the algorithm only reaches Hermitian, positive-semidefinite approximants.

## Optional parameters

| Parameter | Default | Role |
| --- | --- | --- |
| Target distance | `0.5E-13` | Treat input and approximant as equal below this distance. |
| Minimum weight per state | `0` | Drop pure product states below this mixing weight. |
| Target number of states | `N²` | Cap on product states in the mixed-state approximant. |
| Output precision | `3` | Display digits only (`3` / `6` / `9` / `12` / `15`) — not calculation accuracy. |
| Accuracy boost | off | Heavier heuristics (`M = N³`, `R = 1000` vs `M = N²+N`, `R = 100`). See [Time complexity](#time-complexity). |

Build, Docker, and architecture notes for contributors live in [CONTRIBUTING.md](CONTRIBUTING.md).

## Main algorithm

The algorithm is an iterative search for a close approximation to the input matrix **within** the separable matrices subspace.

Starting from the maximally mixed state, the system iteratively adds pure states to the mixed-state approximation. Each pure state is generated to maximize the projection of the distance vector between the current best-approximation matrix and the original input matrix onto the separable matrices subspace.

The main iteration repeats until either a [target distance](#optional-parameters) to the original matrix is reached, or a maximum number of iterations is hit. The heuristic for the maximum number of main iterations is `(target-number-of-states)²` (see target number of states above).

Minimization of the distance between the current best approximation and the original implements a quadratic-programming approach. The equation system is solved efficiently using Eigen's LDLT Cholesky decomposition.

The pure-state collection from which the approximation is constructed is optimized by discarding states with probability below the **Minimum weight per state** threshold.

In each main iteration, constructing the best candidate pure state to mix in is itself an iterative numerical process. The distance between the original matrix and the last approximation is used to generate a tensor-product pure state via a compound adjoint eigenproblem (for each particle). That process refines the pure state until a maximal eigenvalue is reached consistently for all particles, or a heuristic cap is hit (`500` iterations currently).

For background and details, see the original paper: *Geometrical aspects of entanglement*, Physical Review A **74**, 012313 (2006), by Jon Magne Leinaas, Jan Myrheim, and Eirik Ovrum.

## Accuracy

### Input accuracy

Input accuracy is only loosely restricted. Floating-point representation of the data is limited to about 14 significant digits, so entering more than that is redundant. It is also redundant to enter more digits than the chosen [output precision](#optional-parameters), or than the algorithm itself typically delivers (about 6–7 significant digits; see below).

### Output accuracy

Output precision only controls how many decimal digits are **displayed**. It does **not** change calculation time or numerical accuracy.

### Algorithm / calculation accuracy

Because the method is numerical, “accuracy” is not a single well-defined number. Representation is limited by floating-point error (on the order of `E-19`), but searching for pure states and tracing out ideal pure states at each step is an approximation. Heuristics were chosen so that accuracy is good enough while typical-sized inputs finish in a few seconds.

As a concrete check, consider Werner states parametrized by `q`:

```text
W = q*I + (1-q)*B
```

where `I` is the normalized `4×4` maximally mixed state and `B` is any Bell state. For `q < 1/3` the state is separable; for `q > 1/3` it is entangled.

With [accuracy boost mode](#optional-parameters) on, the system finds a separable approximation for `W` within less than `0.5E-13` for `q = 0.3332` (and smaller `q`). For this marginal family, that corresponds to about `0.5E-4` accuracy in the parameter `q`.

## Time complexity

Worst-case time complexity is:

```text
O(R * n * M * N³)
```

where:

| Symbol | Meaning |
| --- | --- |
| `R` | Trace-out refinement steps |
| `M` | Maximum main-iteration steps (see accuracy boost) |
| `N` | Order of the input matrix |
| `n` | Number of particles |

### Details

The initial Peres test applies a single-particle partial transpose per particle. That phase is `O(n * N²)` and is not the limiting cost.

The main algorithm uses heuristics to bound runtime. The main iteration count (how many times we search for an extra pure state to mix in) is limited by `M`, which depends on whether [accuracy boost mode](#optional-parameters) is on, unless another stopping condition is reached first.

**Without** accuracy boost (default):

```text
M = N² + N
R = 100
```

**With** accuracy boost:

```text
M = N³
R = 1000
```

Each main iteration builds a new pure state in several refinement steps over particles (`n`), projecting traced-out parts over each particle’s subspace `O(N²)` and solving for the maximal eigenvalue `O(N³)`. After the new pure state is mixed in, another eigenproblem (QP) finds mixing coefficients in `O(N³)`. A selection sort then orders states by descending probability: `O(targetNumStates * log(targetNumStates))`, which defaults to `O(N³)` unless the user overrides the target count.

So total time complexity is bounded by:

```text
O( (R * n * [N² + N³ + N³]) + N³ ) ≤ O(R * n * N³)
```

## Want to help?

Issues, fixes, and docs improvements are welcome. Start with [CONTRIBUTING.md](CONTRIBUTING.md) and open a GitHub issue or pull request.

## History and credits

Started in 2012 as an undergraduate project at the [Physics Department, Technion – Israel Institute of Technology](https://phys.technion.ac.il/en/), by [Naftaly Shalev](https://www.linkedin.com/in/naftaly-shalev-36b53711a), and completed in 2014 by [Oded Messer](https://github.com/omesser) under the guidance of [J. Avron](https://phsites.technion.ac.il/avron/).

Thanks to [Dr. Oded Kenneth](https://phys.technion.ac.il/en/people/person/295) for mathematical advice at several critical stages.

Bug reports and feedback: [GitHub Issues](https://github.com/omesser/stateseparator/issues).

## License

Distributed under the [GPL-3.0 License](LICENSE). Core logic is C++ (Eigen) with a small HTML/PHP web layer.

Eigen is free software (LGPL3+ / MPL2 in later versions); see the [Eigen license page](https://eigen.tuxfamily.org/index.php?title=Main_Page#License).

Neither the Technion nor the authors are responsible for outcomes from using this program.

*Developed at the Technion – Israel Institute of Technology, Haifa, Israel*

## Appendix: Matrix-element format

Matrix elements are expected in the following forms:

```text
[+/-][real_part][+/-][i][img_part]          Example: +2.3-i0.2
[+/-][real_part][+/-][img_part][i]          Example: -2.3-0.2i
[real_part][+/-][i][img_part]               Example:  2.3-i0.2
[real_part][+/-][img_part][i]               Example:  2.3-0.2i
[+/-][i][img_part][+/-][real_part]          Example: +i2.3-0.2
[+/-][img_part][i][+/-][real_part]          Example: -2.3i-0.2
[i][img_part][+/-][real_part]               Example: i2.3-0.2
[img_part][i][+/-][real_part]               Example: +2.3i+0.2
[+/-][real_part]                            Example: -2.3
[real_part]                                 Example:  2.3
[+/-][img_part][i]                          Example: +2.3i
[+/-][i][img_part]                          Example: -i2.3
[img_part][i]                               Example:  2.3i
[i][img_part]                               Example: i2.3
```

- Note: `i` without a number is interpreted as `1*i`, e.g. `3+i` = `3+1i`.

Elements in a row are separated by one or more whitespace characters; a newline starts a new row.
