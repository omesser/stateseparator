# Quantum State Separator

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](LICENSE)

**Check whether a multi-qudit density matrix is separable or entangled** — and, when it is separable (or close), see a nearest separable approximation and its decomposition into product states.

The tool runs a [Peres–Horodecki](https://en.wikipedia.org/wiki/Peres%E2%80%93Horodecki_criterion) (PPT) partial-transpose test, then a probabilistic numerical search for the nearest separable state based on *Geometrical aspects of entanglement*, Physical Review A **74**, 012313 (2006), by Jon Magne Leinaas, Jan Myrheim, and Eirik Ovrum.

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

Matrix elements may look like `+2.3-i0.2`, `5i`, `-0.2i`, `0.6i-3.1`. See [Matrix element format](docs/matrix-format.md) for the full grammar.

The matrix is checked (to about 3 decimal digits) for Hermitian, positive semidefinite, and unit trace. Warnings do not stop the run, but they affect how meaningful the output is — the algorithm only reaches Hermitian, positive-semidefinite approximants.

### Optional parameters

| Parameter | Default | Role |
| --- | --- | --- |
| Target distance | `0.5E-13` | Treat input and approximant as equal below this distance. |
| Minimum weight per state | `0` | Drop pure product states below this mixing weight. |
| Target number of states | `N²` | Cap on product states in the mixed-state approximant. |
| Output precision | `3` | Display digits only (`3` / `6` / `9` / `12` / `15`) — not calculation accuracy. |
| Accuracy boost | off | Heavier heuristics (`M = N³`, `R = 1000` vs `M = N²+N`, `R = 100`). See [Time complexity](docs/complexity.md). |

## Deeper docs

- [Main algorithm](docs/algorithm.md) — iterative nearest-separable search and quadratic-programming mix-in
- [Accuracy](docs/accuracy.md) — input, display, and calculation accuracy (incl. Werner-state check)
- [Time complexity](docs/complexity.md) — heuristics and big-O
- [Matrix element format](docs/matrix-format.md) — accepted complex-number spellings

Build, Docker, and architecture notes for contributors live in [CONTRIBUTING.md](CONTRIBUTING.md).

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