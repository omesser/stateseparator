# Time complexity

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

## Details

The initial Peres test applies a single-particle partial transpose per particle. That phase is `O(n * N²)` and is not the limiting cost.

The main algorithm uses heuristics to bound runtime. The main iteration count (how many times we search for an extra pure state to mix in) is limited by `M`, which depends on whether [accuracy boost mode](../README.md#optional-parameters) is on, unless another stopping condition is reached first.

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
