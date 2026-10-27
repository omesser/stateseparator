# Bundled third-party code

## Eigen 3.3.9

`Eigen/` is the `Eigen/` folder of upstream Eigen 3.3.9
([tag `3.3.9`](https://gitlab.com/libeigen/eigen/-/tree/3.3.9), released 2020-12-04),
copied unchanged. The one difference: `src/PARDISOSupport/PARDISOSupport.h` is
`src/PardisoSupport/PardisoSupport.h` upstream (only the name's case differs).
Nothing here includes it.

As of 2026-10, Dependabot can't track this copy, so updates are manual. Moving to
3.4 or later needs a check that `tests/test_separator.sh` results still match.
