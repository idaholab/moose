# PolynomialChaos

!syntax description /Surrogates/PolynomialChaos

## Overview

Polynomial chaos is a surrogate modeling technique where a quantity of interest (QoI) that is dependent on input parameters is expanded as a sum of orthogonal polynomials. Given a QoI $Q$ dependent on a set of parameters $\vec{\xi}$, the polynomial chaos expansion (PCE) is:

!equation
Q(\vec{\xi}) = \sum_{i=1}^{P}q_i\Phi_i(\vec{\xi}) ,


where $P$ is the multidimensional polynomial order and $q_i$ are coefficients that are to be computed. These coefficients can be found using intrusive and non intrusive techniques. The intrusive technique is quite difficult to generalize and very computationally demanding. Since the polynomial basis is orthogonal, a non intrusive technique is developed where the coefficients are found by performing a Galerkin projection and integrating:

!equation id=eq:coeff
q_i = \frac{\left\langle Q(\vec{\xi})\Phi_i(\vec{\xi})\right\rangle}{\left\langle\Phi_i(\vec{\xi}),\Phi_i(\vec{\xi})\right\rangle},

where,

!equation
\left\langle a(\vec{\xi}) b(\vec{\xi}) \right\rangle = \int_{-\infty}^{\infty}a(\vec{\xi}) b(\vec{\xi}) f(\vec{\xi}) d\vec{\xi} .


The weight function ($f(\vec{\xi})$) and bases ($\Phi_i(\vec{\xi})$) are typically products of one-dimensional functions:

!equation
f(\vec{\xi}) = \prod_{d=0}^{D}f_d(\xi_d) ,


!equation
Q(\vec{\xi}) = \sum_{i=1}^{P}q_i\prod_{d=0}^{D}\phi^d_{k_{d,i}}(\xi_d) .


The weighting functions are defined by the probability density function of the parameter and the polynomials are based on these distributions, [PolynomialTable] is a list of commonly used distributions and their corresponding orthogonal polynomials.

!table id=PolynomialTable caption=Common probability density functions and their corresponding orthogonal polynomials
| Distribution | Density Function ($f_d(\xi)$) | Polynomial ($\phi_i(\xi)$) | Support|
| - | - | - | - |
| Normal | $\frac{1}{2\pi}e^{-\xi^2/2}$ | Hermite | $[-\infty, \infty]$ |
| Uniform | $\frac{1}{2}$ | Legendre | $[-1, 1]$ |
| Beta | $\frac{(1-\xi)^{\alpha}(1+\xi)^{\beta}}{2^{\alpha+\beta+1}B(\alpha+1,\beta+1)}$ | Jacobi | $[-1,1]$ |
| Exponential | $e^{-\xi}$ | Laguerre | $[0,\infty]$ |
| Gamma | $\frac{\xi^{\alpha}e^{-\xi}}{\Gamma(\alpha+1)}$ | Generalized Laguerre | $[0,\infty]$ |

The expression in [eq:coeff] can be integrated using many different techniques. One is performing a Monte Carlo integration,

!equation
q_i = \frac{1}{\left\langle\Phi_i(\vec{\xi}),\Phi_i(\vec{\xi})\right\rangle} \frac{1}{N_{\mathrm{mc}}}\sum_{n=1}^{N_{\mathrm{mc}}} Q(\vec{\xi}_n)\Phi_i(\vec{\xi}_n) ,


or using numerical quadrature,

!equation
q_i = \frac{1}{\left\langle\Phi_i(\vec{\xi}),\Phi_i(\vec{\xi})\right\rangle}\sum_{n=1}^{N_q} w_n Q(\vec{\xi}_n)\Phi_i(\vec{\xi}_n) .


The numerical quadrature method is typically much more efficient that than the Monte Carlo method and has the added benefit of exactly integrating the polynomial basis. However, the quadrature suffers from the curse of dimensionality. The naive approach uses a Cartesian product of one-dimensional quadratures, which results in $(\max(k^d_i) + 1)^D$ quadrature points to be sampled. Sparse grids can help mitigate the curse of dimensionality significantly.

## Generating a Tuple

In polynomial chaos, a tuple describes the combination of polynomial orders representing the expansion basis ($k_{d,i}$). Again, the naive approach would be to do a tensor product of highest polynomial order, but this is often wasteful since generating a complete monomial basis is usually optimal. Below demonstrates the difference between a tensor basis and a complete monomial basis:

!equation
\begin{aligned}
&D=2, \, k_{\max}=2, \, \text{Tensor product:} \\
&\Phi_0 = \phi^1_0\phi^2_0,\, \Phi_1 = \phi^1_1\phi^2_0,\, \Phi_2 = \phi^1_0\phi^2_1,\, \Phi_3 = \phi^1_1\phi^2_1 \\
&\Phi_4 = \phi^1_2\phi^2_0,\, \Phi_5 = \phi^1_0\phi^2_2,\, \Phi_6 = \phi^1_1\phi^2_2,\, \Phi_7 = \phi^1_2\phi^2_1,\, \Phi_8 = \phi^1_2\phi^2_2 ,
\end{aligned}


!equation
\begin{aligned}
&D=2, \, k_{\max}=2, \, \text{Complete monomial:} \\
&\Phi_0 = \phi^1_0\phi^2_0,\, \Phi_1 = \phi^1_1\phi^2_0,\, \Phi_2 = \phi^1_0\phi^2_1,\, \Phi_3 = \phi^1_1\phi^2_1
\Phi_4 = \phi^1_2\phi^2_0,\, \Phi_5 = \phi^1_0\phi^2_2 .
\end{aligned}


The tuple is generated and stored as matrix in the userobject, below is an example of this matrix with $D=3$ and $k_{\max}=3$:

!equation
k_{d,i} =
\left[
\begin{array}{c|ccc|cccccc|cccccccccc}
0 & 1 & 0 & 0 & 2 & 1 & 1 & 0 & 0 & 0 & 3 & 2 & 2 & 1 & 1 & 1 & 0 & 0 & 0 & 0 \\
0 & 0 & 1 & 0 & 0 & 1 & 0 & 2 & 1 & 0 & 0 & 1 & 0 & 2 & 1 & 0 & 3 & 2 & 1 & 0 \\
0 & 0 & 0 & 1 & 0 & 0 & 1 & 0 & 1 & 2 & 0 & 0 & 1 & 0 & 1 & 2 & 0 & 1 & 2 & 3
\end{array}
\right]


## Computing Statistical Moments

Statistical moments are based on the expectation of a function of the quantity of interest:

!equation
E\left[g\left(Q(\vec{\xi})\right)\right] = \int_{-\infty}^{\infty}g\left(Q(\vec{\xi})\right)f(\vec{\xi})d\vec{\xi} .


The first four statistical moments, and the most common ones, are defined as:

!equation
\text{Mean: } \mu = E[Q] ,

!equation
\text{Variance: } \, \sigma^2 = E\left[\left(Q-\mu\right)^2\right] = E[Q^2] - \mu^2 ,

!equation
\text{Skewness: } \mathrm{Skew} = \frac{E\left[\left(Q-\mu\right)^3\right]}{\sigma^3} = \frac{E[Q^3] - 3\sigma^2\mu - \mu^3}{\sigma^3} ,

!equation
\text{Kurtosis: } \mathrm{Kurt} = \frac{E\left[\left(Q-\mu\right)^4\right]}{\sigma^4} = \frac{E[Q^4] - 4E[Q^3]\mu + 6\sigma^2\mu^2 + 3\mu^4}{\sigma^4} .


Because of the orthogonality of the polynomials, mean and variance are trivial to compute:

!equation
E\left[Q_{\mathrm{pc}}\right] = q_0 ,

!equation
E\left[Q^2_{\mathrm{pc}}\right] = \sum_{i=1}^{P}q_i^2 \prod_{d=1}^{D}\left\langle\phi_{k_{d,i}}^2\right\rangle ,


where $\left\langle\Phi_i^2\right\rangle$ is known analytically. The higher order moments are significantly more taxing to compute since it does not take advantage of orthogonality:

!equation
E\left[Q^3_{\mathrm{pc}}\right] = \sum_{i=1}^{P}\sum_{j=1}^{P}\sum_{k=1}^{P}q_iq_jq_k\prod_{d=1}^{D}\left\langle\phi_{k_{d,i}}\phi_{k_{d,j}}\phi_{k_{d,k}}\right\rangle ,

!equation
E\left[Q^4_{\mathrm{pc}}\right] = \sum_{i=1}^{P}\sum_{j=1}^{P}\sum_{k=1}^{P}\sum_{\ell=1}^{P}q_iq_jq_kq_{\ell}\prod_{d=1}^{D}\left\langle\phi_{k_{d,i}}\phi_{k_{d,j}}\phi_{k_{d,k}}\phi_{k_{d,\ell}}\right\rangle ,


where the polynomial norms are computed using one-dimensional quadrature. We see here the number of operations required to compute Kurtosis is approximately $DN^{4}$. If the number of coefficients is sufficiently high, these moments would probably be best computed inexactly by sampling the PCE surrogate.


## Implementation

The PolynomialChaos user object takes in a list of distributions and constructs a polynomial class based on their type. Given a sampler and a vectorpostprocessor of results from sampling, it then loops through the MC or quadrature points to compute the coefficients. The statistical moments are then computed based on user preferences and the model can be evaluated using the `evaluate` function from any other moose object that has the reference. The algorithm uses the parallelization of the sampler to compute the coefficients, no other part of the algorithm is parallelized.

## Example Input File Syntax

The example involves a homogeneous, one-dimensional diffusion-reaction problem, where the diffusion coefficient ($D$) and reaction coefficient ($\Sigma$) are uncertain with a uniform probability:

!equation
-D\frac{d^2u}{dx^2} + \Sigma u = Q ,

where the QoI is the integrated average of $u(x)$.

After the sub-app is set with the diffusion-reaction problem, distributions are set for each uncertain parameter:

!listing poly_chaos/main_2d_quad.i block=Distributions

A sampler is then defined, either using Monte Carlo,

!listing poly_chaos/main_2d_mc.i block=Samplers Trainers

or quadrature,

!listing poly_chaos/main_2d_quad.i block=Samplers/quadrature

It is important that the order in the quadrature sampler input matches the order in the PolynomialChaos input. The sampler is then used by the MultiApp and Transfers to sample the sub-app, the QoI from the app is then put in a reporter:

!listing poly_chaos/main_2d_quad.i block=Reporters

All this information is ready to be sent to the PolynomialChaos trainer:

!listing poly_chaos/main_2d_quad.i block=Surrogates Trainers

!syntax parameters /Surrogates/PolynomialChaos

!syntax inputs /Surrogates/PolynomialChaos

!syntax children /Surrogates/PolynomialChaos
