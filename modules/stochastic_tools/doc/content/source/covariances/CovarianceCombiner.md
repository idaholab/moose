# CovarianceCombiner

!syntax description /Covariance/CovarianceCombiner

The `CovarianceCombiner` builds composite covariance (kernel) functions by combining
exactly two child kernels via addition (*Sum*) or element-wise multiplication (*Product*).
This follows standard Gaussian Process kernel composition theory, wherein new valid
kernels are constructed from simpler ones to capture multiple sources of structure
in the data simultaneously [!citep](Rasmussen2006gp,Duvenaud2014kernel).

All hyperparameters of both child kernels are exposed through a single flat map and
are tuned *jointly* by the [Gaussian Process Trainer](GaussianProcessTrainer.md).
No additional hyperparameters are introduced by this class itself.

## Sum Operation

The `Sum` operation defines the composite kernel as the pointwise sum of the two
child kernels:

\begin{equation}
    \label{eqn:sum}
    K(\mathbf{x},\mathbf{x}') = K_1(\mathbf{x},\mathbf{x}') + K_2(\mathbf{x},\mathbf{x}')
\end{equation}

A sum kernel models additive contributions to the response. For example, combining
a Squared Exponential kernel (which captures smooth global variation) with a
[LinearCovariance](LinearCovariance.md) kernel (which captures a linear trend) allows
the GP to decouple the trend from the residual structure. Noise from each sub-kernel
adds independently to the diagonal: the effective diagonal noise is
$\sigma_{n,1}^2 + \sigma_{n,2}^2$.

The gradient required by the optimizer follows directly:

\begin{equation}
    \label{eqn:sum_grad}
    \frac{\partial K}{\partial \theta} = \frac{\partial K_1}{\partial \theta} + \frac{\partial K_2}{\partial \theta}
\end{equation}

Because $K_1$ and $K_2$ each own a disjoint set of hyperparameters (distinguished
by their object-name prefix, e.g. `kernel_1:signal_variance`), exactly one term
on the right-hand side is non-zero for any given $\theta$.

## Product Operation

The `Product` operation defines the composite kernel as the element-wise (Hadamard)
product of the two child kernels:

\begin{equation}
    \label{eqn:product}
    K(\mathbf{x},\mathbf{x}') = K_1(\mathbf{x},\mathbf{x}') \odot K_2(\mathbf{x},\mathbf{x}')
\end{equation}

A product kernel can model interactions between independent sources of variation.
For example, multiplying a Squared Exponential kernel with a Matérn kernel applies
the smoothness constraint of both kernels simultaneously, producing a covariance that
decays faster with distance than either kernel alone.

The optimizer gradients follow from the product rule:

\begin{equation}
    \label{eqn:product_grad}
    \frac{\partial K}{\partial \theta_1} = \frac{\partial K_1}{\partial \theta_1} \odot K_2, \qquad
    \frac{\partial K}{\partial \theta_2} = K_1 \odot \frac{\partial K_2}{\partial \theta_2}
\end{equation}

where $K_1$ and $K_2$ are evaluated without the diagonal noise offset
(`is_self_covariance = false`) to remain consistent with how the product kernel
itself is evaluated.

### Note on Noise in Product Kernels

The element-wise product of two noisy kernels does not yield a clean additive
diagonal noise structure:

\begin{equation}
    \label{eqn:product_noise}
    \Big(K_1 + \sigma_{n,1}^2\,\mathbf{I}\Big) \odot \Big(K_2 + \sigma_{n,2}^2\,\mathbf{I}\Big)
    \;\neq\; K_1 \odot K_2 \;+\; \sigma_n^2\,\mathbf{I}
\end{equation}

For this reason, sub-kernels used with `Product` should have `noise_variance = 0`.
Observation noise can be introduced cleanly by wrapping the product kernel inside
an outer `Sum` combiner alongside a pure noise kernel (a
[LinearCovariance](LinearCovariance.md) with `signal_variance = 0` and
`bias_variance = 0`):

\begin{equation}
    \label{eqn:product_noise_fix}
    K_{\text{total}}(\mathbf{x},\mathbf{x}') =
        \underbrace{K_1(\mathbf{x},\mathbf{x}') \odot K_2(\mathbf{x},\mathbf{x}')}_{\text{Product combiner}}
        + \underbrace{\sigma_n^2\,\delta_{\mathbf{x},\mathbf{x}'}}_{\text{noise kernel (Sum combiner)}}
\end{equation}

## Hyperparameter Namespacing

Every hyperparameter is stored with its owning object's name as a prefix
(`object_name:param_name`). This guarantees uniqueness even when both child kernels
share the same parameter name. For example, if $K_1$ is named `rbf` and $K_2$ is
named `lin`, the joint hyperparameter map contains entries such as:
`rbf:signal_variance`, `rbf:length_factor`, `lin:signal_variance`, `lin:c`, and
so on. This namespacing also enables gradient routing: each child kernel's
`computedKdhyper` checks for its own name as a prefix and returns `false` if the
parameter does not belong to it, so calling both sub-kernels in sequence is safe
and correct.

## Recursive Composition

Either child kernel may itself be a `CovarianceCombiner`, enabling arbitrarily deep
kernel trees. Gradient calls propagate down the tree through virtual dispatch without
any additional user configuration. For example, the following tree is valid:

```
top_kernel   : Sum   [ rbf_x_matern,  lin ]
rbf_x_matern : Product [ rbf,  matern ]
```

The optimizer sees a single flat hyperparameter set covering all three leaf kernels
(`rbf`, `matern`, `lin`) and tunes all of them jointly.

## Input Parameters

!table id=ParamTable caption=Input Parameters for CovarianceCombiner
| Parameter | Type | Required | Description |
| - | - | - | - |
| `covariance_functions` | `std::vector<UserObjectName>` | Yes | Names of exactly two child covariance functions defined in the `[Covariance]` block |
| `operation` | `MooseEnum` | Yes | Combination rule: `Sum` ($K = K_1 + K_2$) or `Product` ($K = K_1 \odot K_2$) |

## Example Input File Syntax

### Sum: Squared Exponential + Matérn

The following example combines a Squared Exponential kernel and a Matérn half-integer
kernel additively. Noise lives in `rbf` only to avoid double-counting on the diagonal.

!listing test/tests/surrogates/gaussian_process/GP_rbf_plus_matern.i block=Covariance

### Product: Squared Exponential × Matérn with Noise

The following example multiplies the two kernels and routes observation noise through
a separate pure-noise kernel in an outer `Sum` combiner.

!listing test/tests/surrogates/gaussian_process/GP_rbf_times_matern_with_noise.i block=Covariance

!syntax parameters /Covariance/CovarianceCombiner

!syntax inputs /Covariance/CovarianceCombiner

!syntax children /Covariance/CovarianceCombiner
