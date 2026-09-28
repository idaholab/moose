# KLNormalAux

!syntax description /AuxKernels/KLNormalAux

## Description

`KLNormalAux` sets an auxiliary variable to a sample of the correlated Gaussian random field
$Z(\mathbf{p})$ of a [KLExpansionUserObject](KLExpansionUserObject.md), shifted by a constant,

\begin{equation}
u(\mathbf{p}) = \mu + Z(\mathbf{p}),
\end{equation}

where $\mu$ is [!param](/AuxKernels/KLNormalAux/mean) and the covariance of $Z$ is defined by the
covariance objects of the expansion named in
[!param](/AuxKernels/KLNormalAux/kl_user_object). Nodal variables are evaluated at the nodes and
elemental variables at the quadrature points. Because the expansion is truncated, the pointwise
variance of the field is slightly smaller than the nominal variance, most noticeably near the
boundary of the expansion's reference box. The field is unbounded, so it is not suited to
quantities that must remain positive; use [KLWeibullAux](KLWeibullAux.md) or
[SigmoidTrendWeibullAux](SigmoidTrendWeibullAux.md) for those. See [random_fields.md] for the
theory.

## Example Input Syntax

In this example, the same realization of a two-dimensional field with a mean of 1 is written to a
nodal and to an elemental variable.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/normal_2d.i block=AuxKernels

!syntax parameters /AuxKernels/KLNormalAux

!syntax inputs /AuxKernels/KLNormalAux

!syntax children /AuxKernels/KLNormalAux
