# KLExpansionUserObject

!syntax description /UserObjects/KLExpansionUserObject

## Description

`KLExpansionUserObject` builds a truncated Karhunen-Loeve expansion of a zero-mean Gaussian random
field in one to three dimensions, with a covariance that is the product of one-dimensional
covariance kernels, and evaluates one realization of the field at arbitrary points. It is sampled
by [KLNormalAux](KLNormalAux.md), [KLWeibullAux](KLWeibullAux.md), and
[SigmoidTrendWeibullAux](SigmoidTrendWeibullAux.md). The theory is described in
[random_fields.md].

The number of dimensions of the field is the length of
[!param](/UserObjects/KLExpansionUserObject/lower_bounds), and
[!param](/UserObjects/KLExpansionUserObject/upper_bounds),
[!param](/UserObjects/KLExpansionUserObject/n_grid), and
[!param](/UserObjects/KLExpansionUserObject/covariance_functions) must have the same length, with
entries in the order of the coordinate directions. The bounds define a box that should enclose the
mesh. In each direction, the named covariance, either
[KLExponentialCovariance](KLExponentialCovariance.md) or
[KLSquaredExponentialCovariance](KLSquaredExponentialCovariance.md), is discretized on a uniform
reference grid of [!param](/UserObjects/KLExpansionUserObject/n_grid) points, and the resulting
eigenvectors are extended to arbitrary coordinates with the Nystrom method. The grid spacing
should be several times smaller than the covariance length scale in that direction.

The expansion is truncated by exactly one of two parameters, and an error is reported if both or
neither are given:

- [!param](/UserObjects/KLExpansionUserObject/n_terms) retains a fixed number of joint modes with
  the largest eigenvalues. A warning is issued, and all modes are retained, if it exceeds the
  number of available modes.
- [!param](/UserObjects/KLExpansionUserObject/variance_fraction) retains the fewest joint modes
  whose eigenvalues sum to at least this fraction of the total.

All candidate joint modes are stored and sorted before truncation, so memory use grows with the
product of the numbers of marginal modes in each direction; see the memory warning in
[random_fields.md#practical-guidance].

The random coefficients of the retained modes are drawn from standard normal distributions using
[!param](/UserObjects/KLExpansionUserObject/seed). Each seed defines one realization, which does
not depend on the number of processors. The expansion and its coefficients are computed once,
when the object is constructed.

## Example Input Syntax

In this example, a two-dimensional field on the unit square uses the same squared-exponential
covariance, with a length scale of 0.2, in both directions. Each direction is discretized with 40
reference grid points, and the expansion keeps the fewest terms that capture 99% of the variance.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/normal_2d.i block=UserObjects

In this example, a three-dimensional field is anisotropic because the $y$ direction uses an
exponential covariance with a longer length scale than the squared-exponential covariances in the
$x$ and $z$ directions.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/normal_3d.i block=UserObjects

!syntax parameters /UserObjects/KLExpansionUserObject

!syntax inputs /UserObjects/KLExpansionUserObject

!syntax children /UserObjects/KLExpansionUserObject
