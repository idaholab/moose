# PolynomialChaosLocalSensitivity

This vector postprocessor uses a trained [PolynomialChaos.md] surrogate and computes local sensitivities of a quantity of interest $u$, for a parameter $\xi_p$ at a point ($\vec{\xi}$) is defined as:

!equation
S_p = \frac{\partial u(\vec{\xi})}{\partial \xi_p} \frac{\xi_p}{u(\vec{\xi})}

## Example Syntax

!listing poly_chaos/master_2d_quad_locs.i block=VectorPostprocessors/local_sense

As shown above, sample points ($\vec{\xi}$) can be defined by either inputting them as a vector directly, or using a sampler defined like:

!listing poly_chaos/master_2d_quad_locs.i block=Samplers/grid

The parameter with which to take the sensitivity with respect to is defined by inputting the variable numbers in [!param](/VectorPostprocessors/PolynomialChaosLocalSensitivity/sensitivity_parameters), which will correspond to the order of distributions set up by the training sampler. If this input is empty or ignored, the sensitivity with respect to every parameter is calculated.

!syntax parameters /VectorPostprocessors/PolynomialChaosLocalSensitivity

!syntax inputs /VectorPostprocessors/PolynomialChaosLocalSensitivity

!syntax children /VectorPostprocessors/PolynomialChaosLocalSensitivity
