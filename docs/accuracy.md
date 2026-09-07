# Accuracy

## Input accuracy

Input accuracy is only loosely restricted. Floating-point representation of the data is limited to about 14 significant digits, so entering more than that is redundant. It is also redundant to enter more digits than the chosen [output precision](../README.md#optional-parameters), or than the algorithm itself typically delivers (about 6–7 significant digits; see below).

## Output accuracy

Output precision only controls how many decimal digits are **displayed**. It does **not** change calculation time or numerical accuracy.

## Algorithm / calculation accuracy

Because the method is numerical, “accuracy” is not a single well-defined number. Representation is limited by floating-point error (on the order of `E-19`), but searching for pure states and tracing out ideal pure states at each step is an approximation. Heuristics were chosen so that accuracy is good enough while typical-sized inputs finish in a few seconds.

As a concrete check, consider Werner states parametrized by `q`:

```text
W = q*I + (1-q)*B
```

where `I` is the normalized `4×4` maximally mixed state and `B` is any Bell state. For `q < 1/3` the state is separable; for `q > 1/3` it is entangled.

With [accuracy boost mode](../README.md#optional-parameters) on, the system finds a separable approximation for `W` within less than `0.5E-13` for `q = 0.3332` (and smaller `q`). For this marginal family, that corresponds to about `0.5E-4` accuracy in the parameter `q`.
