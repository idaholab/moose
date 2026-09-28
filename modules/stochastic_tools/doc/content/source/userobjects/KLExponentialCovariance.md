# KLExponentialCovariance

!syntax description /UserObjects/KLExponentialCovariance

## Description

`KLExponentialCovariance` defines the one-dimensional covariance kernel

\begin{equation}
C_k(x_1, x_2) = \sigma_k^2 \exp \left( -\frac{|x_1 - x_2|}{\ell_k} \right),
\end{equation}

where $\sigma_k^2$ is [!param](/UserObjects/KLExponentialCovariance/variance) and $\ell_k$ is
[!param](/UserObjects/KLExponentialCovariance/length_scale). It supplies the covariance in one direction
of a [KLExpansionUserObject](KLExpansionUserObject.md), whose covariance is the product of the
kernels listed in [!param](/UserObjects/KLExpansionUserObject/covariance_functions), so the
variance of the field is the product of the variances of its kernels. See [random_fields.md] for
the theory.

The exponential covariance produces fields that are continuous but not differentiable, and its
eigenvalues decay slowly, so more expansion terms are needed than for a
[KLSquaredExponentialCovariance](KLSquaredExponentialCovariance.md) with the same length scale.

## Example Input Syntax

In this example, the exponential covariance defines a one-dimensional field with a variance of 0.5
and a length scale of 0.4.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/normal_1d.i block=UserObjects

!syntax parameters /UserObjects/KLExponentialCovariance

!syntax inputs /UserObjects/KLExponentialCovariance

!syntax children /UserObjects/KLExponentialCovariance
