# PolynomialChaosReporter

!syntax description /Reporters/PolynomialChaosReporter

## Overview

This object is meant to compute relevant statistics and sensitivities from a [PolynomialChaos.md] surrogate and output information about the model. Users can specify multiple models in the [!param](/Reporters/PolynomialChaosReporter/pc_name) parameter.

### Statistics

To compute statistics from a [PolynomialChaos.md] surrogate, use the [!param](/Reporters/PolynomialChaosReporter/statistics) parameters. So far, mean, standard deviation, skewness, and kurtosis can all be computed. See [PolynomialChaos.md] for more details on the calculation of these statistics. The output from this computation is largely identical to [StatisticsReporter.md].

!listing surrogates/poly_chaos/main_2d_quad_moment.i block=Reporters

!listing surrogates/poly_chaos/gold/main_2d_quad_moment_out.json language=json

### Sobol Sensitivity

Setting the [!param](/Reporters/PolynomialChaosReporter/include_sobol) to `true` will compute sobol indices from the inputted polynomial chaos models. The alogrithm is based on computations described in [!cite](sudret2008global). The object will compute total, first-, and second-order indices. The output is largely indentical to [SobolReporter.md].

!listing poly_chaos/sobol.i block=Reporters

!listing poly_chaos/gold/sobol_out.json language=json

### Local Sensitivity

Users can compute local sensitivities with this object by including the [!param](/Reporters/PolynomialChaosReporter/local_sensitivity_points) and/or [!param](/Reporters/PolynomialChaosReporter/local_sensitivity_sampler) parameters. The local sensitivity of a quantity of interest $u$, for a parameter $\xi_p$ at a point ($\vec{\xi}$) is defined as:

!equation
S_p = \frac{\partial u(\vec{\xi})}{\partial \xi_p} \frac{\xi_p}{u(\vec{\xi})}

For each inputted model, the output will contain a two matrix reporter value corresponding to the points specified by [!param](/Reporters/PolynomialChaosReporter/local_sensitivity_points) and [!param](/Reporters/PolynomialChaosReporter/local_sensitivity_sampler). The row of the matrix corresponds to the point and column corresponds to the derivative with respect to the parameter $\xi_p$.

!listing surrogates/poly_chaos/main_2d_quad_locs.i block=Reporters

!listing surrogates/poly_chaos/gold/main_2d_quad_locs_out.json language=json

### Model Data

Users can output the information on the models inputted by setting the [!param](/Reporters/PolynomialChaosReporter/include_data) parameter to `true`.

!listing surrogates/load_store/evaluate.i block=Reporters

!listing surrogates/load_store/gold/evaluate_out.json language=json

!syntax parameters /Reporters/PolynomialChaosReporter

!syntax inputs /Reporters/PolynomialChaosReporter

!syntax children /Reporters/PolynomialChaosReporter
