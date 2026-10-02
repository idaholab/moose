# LinearCovariance

!syntax description /Covariance/LinearCovariance

## Overview

The linear covariance function is a *non-stationary* kernel for Gaussian Processes.
Unlike stationary kernels (e.g. Squared Exponential or Matérn), the covariance between
two points depends on their absolute positions in input space, not only on the
distance between them. The kernel is defined as:

!equation
K(\mathbf{x},\mathbf{x}') = \sigma_b^2 + \sigma_v^2 \sum_{k=1}^{n} (x_k - c_k)(x'_k - c_k) + \sigma_n^2\,\delta_{\mathbf{x},\mathbf{x}'}

where $n$ is the number of input dimensions. The Kronecker delta $\delta_{\mathbf{x},\mathbf{x}'}$
is applied only to the diagonal of the covariance matrix when evaluating the
self-covariance (training points against themselves).

The offset vector $\mathbf{c}$ sets the *pivot point* of the kernel — the location
in input space through which all linear basis functions pass. Setting
$\mathbf{c} = \mathbf{0}$ and $\sigma_b^2 = 0$ recovers the plain dot-product kernel:

!equation
K(\mathbf{x},\mathbf{x}') = \sigma_v^2\,\mathbf{x}^{\intercal}\mathbf{x}'

## Non-Stationarity

A kernel is *stationary* when $K(\mathbf{x}, \mathbf{x}')$ depends only on the
difference $\mathbf{x} - \mathbf{x}'$. The linear kernel does not satisfy this
condition because $(x_k - c_k)(x'_k - c_k)$ involves each point's coordinates
individually rather than their difference. As a consequence, the prior variance
at a point $\mathbf{x}$ grows with its distance from the pivot $\mathbf{c}$:

!equation
K(\mathbf{x},\mathbf{x}) = \sigma_b^2 + \sigma_v^2\,\|\mathbf{x} - \mathbf{c}\|^2 + \sigma_n^2

This makes the linear kernel well-suited for responses that exhibit a global linear
or affine trend across the input domain, such as quantities of interest that scale
monotonically with one or more input parameters. When paired with a stationary
kernel through [CovarianceCombiner](CovarianceCombiner.md) using the `Sum` operation,
the linear kernel captures the global trend while the stationary kernel captures
residual local structure.

## Hyperparameters

!table id=HyperparamTable caption=Hyperparameters for the Linear Covariance Function
| Variable | Domain | Tunable | Description |
| - | - | - | - |
| $\mathbf{c}$ | $\mathbb{R}^n$ | Yes | Offset (pivot point) per input dimension; one entry per input parameter |
| $\sigma_v$ | $\mathbb{R}_{\geq 0}$ | Yes | Signal variance; scales the inner product term\* |
| $\sigma_b$ | $\mathbb{R}_{\geq 0}$ | Yes | Bias variance; constant offset added to all covariance entries\* |
| $\sigma_n$ | $\mathbb{R}_{\geq 0}$ | Yes | Noise variance; added to the diagonal for self-covariance\* |

\*See the [Gaussian Process Trainer](GaussianProcessTrainer.md) documentation for
a more in-depth explanation of $\sigma_v$, $\sigma_b$, and $\sigma_n$.

## Hyperparameter Gradients

The [Gaussian Process Trainer](GaussianProcessTrainer.md) requires analytic gradients
$\partial K / \partial \theta$ for each tunable hyperparameter $\theta$. All four
gradients are computed in closed form:

!equation
\frac{\partial K}{\partial \sigma_v^2} = \sum_{k=1}^{n}(x_k - c_k)(x'_k - c_k)

!equation
\frac{\partial K}{\partial \sigma_b^2} = 1

!equation
\frac{\partial K}{\partial \sigma_n^2} = \delta_{\mathbf{x},\mathbf{x}'}

!equation
\frac{\partial K}{\partial c_d} = -\sigma_v^2\Big[(x_d - c_d) + (x'_d - c_d)\Big]

The gradient with respect to $c_d$ follows from the product rule applied to the
single dimension $d$; all other dimensions are independent of $c_d$ and vanish.

## Example Input File Syntax

The following example trains a Gaussian Process surrogate on two input parameters
($k$ and $q$) using the linear covariance function. The offset $\mathbf{c}$ is
initialized to the midpoint of each parameter's domain.

!listing test/tests/surrogates/gaussian_process/GP_linear.i block=Covariance

!syntax parameters /Covariance/LinearCovariance

!syntax inputs /Covariance/LinearCovariance

!syntax children /Covariance/LinearCovariance
