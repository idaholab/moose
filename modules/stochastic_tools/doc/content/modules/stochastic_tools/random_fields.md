# Random Fields from a Separable Karhunen-Loeve Expansion

The stochastic tools module can populate auxiliary variables with spatially correlated random
fields, for example to represent the variability of a material property such as Young's modulus.
A Gaussian field is built from a truncated Karhunen-Loeve (KL) expansion
[!citep](ghanem1991stochastic) of a covariance function that is separable across the coordinate
directions. Separability replaces one large eigenproblem over the domain with one small
eigenproblem per direction, and the Nystrom extension
[!citep](nystrom1930praktische, williams2000using) evaluates the expansion at arbitrary points,
such as the nodes of an unstructured mesh. Non-Gaussian fields with the same correlation
structure are obtained from the Gaussian field through a Gaussian copula.

## Workflow

Three kinds of objects are combined:

1. One one-dimensional covariance [UserObject](UserObjects/index.md) per direction of the field,
   either [KLExponentialCovariance](KLExponentialCovariance.md) or
   [KLSquaredExponentialCovariance](KLSquaredExponentialCovariance.md). Each direction may use a
   different kernel, variance, or length scale to make the field anisotropic.
1. One [KLExpansionUserObject](KLExpansionUserObject.md), which references the covariance
   objects, builds the truncated expansion over a box that encloses the mesh, and samples the
   random coefficients from its seed.
1. One [AuxKernel](AuxKernels/index.md) per sampled variable:

   - [KLNormalAux](KLNormalAux.md) for the Gaussian field,
   - [KLWeibullAux](KLWeibullAux.md) for a Weibull field, or
   - [SigmoidTrendWeibullAux](SigmoidTrendWeibullAux.md) for a Weibull field whose scale varies
     with distance from a line segment.

   Several auxiliary kernels can sample the same `KLExpansionUserObject`, in which case the
   resulting fields are transformations of the same Gaussian realization.

In this example, two isotropic squared-exponential covariances with unit variance and a length
scale of 0.2 define the field on the unit square. The expansion retains the fewest terms that
capture 99% of the variance, and the same realization is written to a Weibull field with a
constant scale and to one whose scale decreases away from the vertical centerline $x = 0.5$.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/weibull_2d.i

The sampled variables can then be coupled into materials in the usual way, for example through
a [ParsedMaterial](ParsedMaterial.md) that supplies a spatially varying Young's modulus to an
elasticity tensor.

## Separable Karhunen-Loeve Expansion

Let $\mathbf{p} = (p_0, \dots, p_{d-1})$, $d \le 3$, denote a spatial point. The covariance
between two points $\mathbf{p}$ and $\mathbf{q}$ is taken to be separable,

\begin{equation}
C(\mathbf{p}, \mathbf{q}) = \prod_{k=0}^{d-1} C_k(p_k, q_k),
\end{equation}

where each $C_k$ is a one-dimensional covariance kernel. The KL expansion of a zero-mean Gaussian
field with this covariance is

\begin{equation}
\label{eq:kl}
Z(\mathbf{p}) = \sum_{i=1}^\infty \sqrt{\lambda_i}\,\varphi_i(\mathbf{p})\,\xi_i,
\qquad \xi_i \overset{\text{i.i.d.}}{\sim} \mathcal{N}(0,1),
\end{equation}

where $\lambda_i$ and $\varphi_i$ are the eigenvalues and orthonormal eigenfunctions of the
Fredholm integral equation of the second kind

\begin{equation}
\int_\Omega C(\mathbf{p}, \mathbf{q})\, \varphi_i(\mathbf{q})\, d\mathbf{q}
  = \lambda_i\, \varphi_i(\mathbf{p}).
\end{equation}

On a box $\Omega = \Omega_0 \times \dots \times \Omega_{d-1}$, set by
[!param](/UserObjects/KLExpansionUserObject/lower_bounds) and
[!param](/UserObjects/KLExpansionUserObject/upper_bounds), the separable covariance splits this
problem into $d$ independent one-dimensional integral equations,

\begin{equation}
\int_{\Omega_k} C_k(p_k, q_k)\, \varphi^{(k)}_{m}(q_k)\, dq_k
  = \lambda^{(k)}_m\, \varphi^{(k)}_m(p_k), \qquad k = 0,\dots,d-1,
\end{equation}

and the joint eigenpairs are products of the marginal ones,

\begin{equation}
\varphi_{\mathbf{m}}(\mathbf{p}) = \prod_{k=0}^{d-1} \varphi^{(k)}_{m_k}(p_k), \qquad
\Lambda_{\mathbf{m}} = \prod_{k=0}^{d-1} \lambda^{(k)}_{m_k},
\end{equation}

indexed by the multi-index $\mathbf{m} = (m_0, \dots, m_{d-1})$.

## Discretization and Nystrom Extension

Each one-dimensional integral equation is discretized on a uniform reference grid
$\{t^{(k)}_1, \dots, t^{(k)}_{n_k}\}$ with spacing $h_k$, where $n_k$ is set by
[!param](/UserObjects/KLExpansionUserObject/n_grid). This gives the symmetric matrix eigenproblem

\begin{equation}
\mathbf{C}^{(k)}\mathbf{v}^{(k)}_m = \mu^{(k)}_m \mathbf{v}^{(k)}_m, \qquad
\mathbf{C}^{(k)}_{ij} = C_k\left(t^{(k)}_i, t^{(k)}_j\right),
\end{equation}

with unit-norm eigenvectors $\mathbf{v}^{(k)}_m$. The matrix eigenpairs approximate the continuous
ones through $\lambda^{(k)}_m \approx h_k \mu^{(k)}_m$ and
$\varphi^{(k)}_m(t^{(k)}_l) \approx v^{(k)}_{m,l} / \sqrt{h_k}$. The eigenfunction at an arbitrary
coordinate $s$ follows from substituting the discrete solution back into the integral equation,
which is the Nystrom extension. Scaled by the square root of its eigenvalue, the extended mode is

\begin{equation}
\label{eq:nystrom}
\psi^{(k)}_m(s) \equiv \sqrt{\lambda^{(k)}_m}\,\varphi^{(k)}_m(s)
  \approx \frac{1}{\sqrt{\mu^{(k)}_m}} \sum_{l=1}^{n_k} C_k\left(s, t^{(k)}_l\right) v^{(k)}_{m,l}.
\end{equation}

The grid spacing cancels, so the scaled modes depend only on the matrix eigenpairs. At the grid
points they reduce to $\psi^{(k)}_m(t^{(k)}_l) = \sqrt{\mu^{(k)}_m}\, v^{(k)}_{m,l}$, and the
expansion reproduces the covariance matrix to roundoff when all modes are retained. Away from the grid
points, the extension interpolates with the covariance kernel itself and converges to the
continuous eigenfunctions as $n_k$ increases. Marginal eigenvalues at or below the roundoff level
of the eigensolver, $n_k \epsilon \mu^{(k)}_{\max}$ with $\epsilon$ the machine precision, are
discarded because their eigenvectors carry no information.

## Truncation

The candidate joint eigenvalues $\Lambda_{\mathbf{m}} = \prod_k \mu^{(k)}_{m_k}$ are sorted in
descending order, and exactly one of two rules selects the retained set $\mathcal{M}$:

- [!param](/UserObjects/KLExpansionUserObject/n_terms) retains the $M$ leading joint modes.
- [!param](/UserObjects/KLExpansionUserObject/variance_fraction) $f \in (0, 1]$ retains the
  fewest leading modes whose eigenvalues sum to at least the fraction $f$ of the total trace,

  \begin{equation}
  \frac{\sum_{i=1}^{|\mathcal{M}|} \Lambda_{(i)}}{\sum_{\mathbf{m}} \Lambda_{\mathbf{m}}} \ge f.
  \end{equation}

  Because the number of retained modes adapts to the covariance, this rule is easier to use when
  comparing different length scales.

The sampled Gaussian field is

\begin{equation}
\label{eq:field}
Z(\mathbf{p}) = \sum_{\mathbf{m} \in \mathcal{M}} a_{\mathbf{m}}(\mathbf{p})\, \xi_{\mathbf{m}},
\qquad
a_{\mathbf{m}}(\mathbf{p}) = \prod_{k=0}^{d-1} \psi^{(k)}_{m_k}(p_k),
\end{equation}

where the standard normal coefficients $\xi_{\mathbf{m}}$ are drawn from
[!param](/UserObjects/KLExpansionUserObject/seed) in the order of the sorted modes, with ties
between equal eigenvalues broken by the marginal mode indices. A realization therefore depends only
on the seed and the expansion, not on the number of processors or on the mesh partitioning, and
increasing the number of retained terms with the same seed refines the same realization rather
than drawing a new one.

[!ref](fig-normal) shows Gaussian fields on the unit square for several length scales of a
squared-exponential covariance.

!media stochastic_tools/random_fields/normal_random_field.png
       style=width:100%;
       id=fig-normal
       caption=Gaussian random fields with a squared-exponential covariance of unit variance and a
       length scale of (a) 0.05, (b) 0.1, and (c) 0.4.

## Gaussian Copula

A non-Gaussian field with the same correlation structure is obtained by passing the Gaussian field
through a Gaussian copula [!citep](nelsen2006introduction) and an inverse cumulative distribution
function. Because the expansion is truncated, its pointwise variance

\begin{equation}
\sigma^2(\mathbf{p}) = \sum_{\mathbf{m} \in \mathcal{M}} a_{\mathbf{m}}(\mathbf{p})^2
\end{equation}

is smaller than the nominal variance $\prod_k \sigma_k^2$ and varies from point to point. It is
lowest near the boundary of the reference box, where the retained modes represent the field least
well. The copula requires a standard normal value at every point, so the field is first
standardized pointwise,

\begin{equation}
\widetilde{Z}(\mathbf{p}) = \frac{Z(\mathbf{p})}{\sigma(\mathbf{p})},
\end{equation}

with $\widetilde{Z}(\mathbf{p}) = 0$ where $\sigma(\mathbf{p}) < 10^{-14}$, for example far outside
the reference box. This rescales each point individually and leaves the correlation between any
two points unchanged. The standard normal cumulative distribution function $\Phi$ then gives a
field that is uniformly distributed on $(0, 1)$ at every point,

\begin{equation}
U(\mathbf{p}) = \Phi\left(\widetilde{Z}(\mathbf{p})\right)
  = \frac{1}{2}\left[1 + \operatorname{erf}\left(\frac{\widetilde{Z}(\mathbf{p})}{\sqrt{2}}\right)\right],
\end{equation}

which is limited to $[10^{-12}, 1 - 10^{-12}]$ so that inverse cumulative distribution functions
with unbounded support remain finite. Applying the inverse cumulative distribution function of the
target distribution to $U(\mathbf{p})$ yields a field with that marginal distribution and the
dependence structure of the Gaussian field. [KLWeibullAux](KLWeibullAux.md) and
[SigmoidTrendWeibullAux](SigmoidTrendWeibullAux.md) use the Weibull distribution
[!citep](weibull1951statistical).

!media stochastic_tools/random_fields/normal_vs_weibull.png
       style=width:100%;
       id=fig-weibull
       caption=(a) Gaussian random field and (b) Weibull random field obtained from it through the
       Gaussian copula, both with a squared-exponential covariance of unit variance and a length
       scale of 0.1.

[!ref](fig-weibull) compares a Gaussian field with the Weibull field obtained from it. The spatial
structure is preserved, while the values follow the Weibull distribution.

## Practical Guidance

- The reference box should enclose the mesh. Beyond the box, the extended modes decay with the
  covariance kernel and the field loses variance.
- The reference grid spacing should be several times smaller than the length scale in each
  direction so that the retained modes are resolved.
- The cost of evaluating the field at a point grows with the number of reference grid points
  times the number of marginal modes used in each direction, plus the number of retained joint
  modes. Short length scales in three dimensions can require thousands of joint modes.
- Use [KLWeibullAux](KLWeibullAux.md) or [SigmoidTrendWeibullAux](SigmoidTrendWeibullAux.md),
  not [KLNormalAux](KLNormalAux.md), for quantities that must remain positive, such as an elastic
  modulus.
