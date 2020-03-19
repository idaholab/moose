# PolynomialChaosStatistics

This object is designed to output statistically relevant quantities from a trained polynomial chaos expansion. So far, mean, standard deviation, skewness, and kurtosis can all be computed using this postprocessor. See [PolynomialChaos.md] for more details on the calculation of these statistics.

## Example Syntax

!listing poly_chaos/master_2d_quad_moment.i block=VectorPostprocessors/pc_moments

!syntax parameters /VectorPostprocessors/PolynomialChaosStatistics

!syntax inputs /VectorPostprocessors/PolynomialChaosStatistics

!syntax children /VectorPostprocessors/PolynomialChaosStatistics
