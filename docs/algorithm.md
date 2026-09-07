# Main algorithm

The algorithm is an iterative search for a close approximation to the input matrix **within** the separable matrices subspace.

Starting from the maximally mixed state, the system iteratively adds pure states to the mixed-state approximation. Each pure state is generated to maximize the projection of the distance vector between the current best-approximation matrix and the original input matrix onto the separable matrices subspace.

The main iteration repeats until either a [target distance](../README.md#optional-parameters) to the original matrix is reached, or a maximum number of iterations is hit. The heuristic for the maximum number of main iterations is `(target-number-of-states)²` (see target number of states in the README).

Minimization of the distance between the current best approximation and the original implements a quadratic-programming approach. The equation system is solved efficiently using Eigen's LDLT Cholesky decomposition.

The pure-state collection from which the approximation is constructed is optimized by discarding states with probability below the **Minimum weight per state** threshold.

In each main iteration, constructing the best candidate pure state to mix in is itself an iterative numerical process. The distance between the original matrix and the last approximation is used to generate a tensor-product pure state via a compound adjoint eigenproblem (for each particle). That process refines the pure state until a maximal eigenvalue is reached consistently for all particles, or a heuristic cap is hit (`500` iterations currently).

For background and details, see the original paper: *Geometrical aspects of entanglement*, Physical Review A **74**, 012313 (2006), by Jon Magne Leinaas, Jan Myrheim, and Eirik Ovrum.

See also [Accuracy](accuracy.md) and [Time complexity](complexity.md).
