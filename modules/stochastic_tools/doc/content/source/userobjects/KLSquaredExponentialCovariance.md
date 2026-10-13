# KLSquaredExponentialCovariance

!syntax description /UserObjects/KLSquaredExponentialCovariance

## Description

`KLSquaredExponentialCovariance` defines the one-dimensional covariance kernel

\begin{equation}
C_k(x_1, x_2) = \sigma_k^2 \exp \left( -\frac{(x_1 - x_2)^2}{2 \ell_k^2} \right),
\end{equation}

where $\sigma_k^2$ is [!param](/UserObjects/KLSquaredExponentialCovariance/variance) and $\ell_k$ is
[!param](/UserObjects/KLSquaredExponentialCovariance/length_scale). It supplies the covariance in one direction
of a [KLExpansionUserObject](KLExpansionUserObject.md), whose covariance is the product of the
kernels listed in [!param](/UserObjects/KLExpansionUserObject/covariance_functions), so the
variance of the field is the product of the variances of its kernels. See [random_fields.md] for
the theory.

The squared-exponential covariance produces smooth fields, and its eigenvalues decay rapidly, so
few expansion terms are needed compared with a
[KLExponentialCovariance](KLExponentialCovariance.md) with the same length scale.

## Example Input Syntax

In this example, the same squared-exponential covariance, with unit variance and a length scale
of 0.2, is used in both directions of a two-dimensional field.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/normal_2d.i block=UserObjects

!syntax parameters /UserObjects/KLSquaredExponentialCovariance

!syntax inputs /UserObjects/KLSquaredExponentialCovariance

!syntax children /UserObjects/KLSquaredExponentialCovariance
