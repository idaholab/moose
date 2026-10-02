# KLWeibullAux

!syntax description /AuxKernels/KLWeibullAux

## Description

`KLWeibullAux` sets an auxiliary variable to a spatially correlated random field with a Weibull
marginal distribution [!citep](weibull1951statistical). The correlated Gaussian field of the
[KLExpansionUserObject](KLExpansionUserObject.md) named in
[!param](/AuxKernels/KLWeibullAux/kl_user_object) is standardized pointwise and mapped through the
standard normal cumulative distribution function to a correlated field $U(\mathbf{p})$ that is
uniform on $(0, 1)$ at every point, which is a Gaussian copula. The inverse Weibull cumulative
distribution function then gives

\begin{equation}
W(\mathbf{p}) = \lambda \left[ -\ln \left( 1 - U(\mathbf{p}) \right) \right]^{1/k},
\end{equation}

where $\lambda$ is [!param](/AuxKernels/KLWeibullAux/scale) and $k$ is
[!param](/AuxKernels/KLWeibullAux/shape). The scale sets the characteristic magnitude of the
field, and larger shape values give a narrower distribution about it. The field is positive and
inherits the spatial dependence structure of the Gaussian field. Nodal variables are evaluated at
the nodes and elemental variables at the quadrature points. See [random_fields.md] for the theory
and for a comparison of Gaussian and Weibull fields.

## Example Input Syntax

In this example, a Weibull field with a shape of 8 and a scale of 10 is sampled from a
two-dimensional expansion.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/weibull_2d.i
         block=AuxKernels/weibull

!syntax parameters /AuxKernels/KLWeibullAux

!syntax inputs /AuxKernels/KLWeibullAux

!syntax children /AuxKernels/KLWeibullAux
