# QuadratureSampler

!syntax description /Samplers/QuadratureSampler

## Overview

Numerical quadrature is a method of selecting specific points and weights in integrate a function,

\begin{equation}
\int_{a}^{b}y(x)f(x)dx = \sum_{q=1}^{N_q}w_qy(x_q) ,
\end{equation}

where $y(x)$ is the function being integrated, $f(x)$ is weighting function, $N_q$ is the number of quadrature points, $w_q$ and $x_q$ are quadrature weights and points, respectively. The sampler generates the points and weights based on the weighting function and the desired integration order. A uniform weighting function uses Gauss-Legendre quadrature and a normal weighting function uses Gauss-Hermite quadrature. These two quadratures can exactly integrate a polynomial of order $2N_q-1$.

For multidimensional quadratures, the following options are available by setting [!param](/Samplers/QuadratureSampler/sparse_grid):

- `none` (default): Tensor grid
- `smolyak`: Smolyak sparse grid
- `clenshaw-curtis`: Clenshaw-Curtis sparse grid

In general, a multi-dimensional quadrature grid is a combination of 1-D quadratures. For example, a tensor grid can be describe as

!equation
\mathcal{A}(D,N_q) = Q_1^{N_q} \otimes\dots\otimes Q_D^{N_q} ,

where $D$ is the number of dimensions and $Q_d^{j}$ is the one-dimension quadrature set for dimension $d$ with $j$ points. In contrast, the Smolyak and Clenshaw-Curtis grid is described as:

!equation
\mathcal{A}(D,N_q) = \sum_{N_q-D \leq |\bm{i}| \leq N_q-1} (-1)^{N_q+D-|\bm{i}|-1} \binom{N_q+D-1}{N_q+D-|\bm{i}|-1} \left(Q_1^{i_1} \otimes\dots\otimes Q_D^{i_D}\right) ,

where $|\bm{i}| = \sum_{d=1}^D i_d$. See [!cite](gerstner1998numerical) for more details regarding sparse grids.

### Tensor Grid

A tensor grid is created created by taking a Cartesian product of the points and a Kroenecker product of the weights from each dimensions' full order quadrature set. The resulting number of points is $N_q^D$, where $D$ is the number of dimensions. Currently, only Gauss quadrature types are used with tensor grid. [tensor-npts] shows the number of points several different $D$ and $N_q$. [tensor-grid] shows the points of a tensor grid of Gauss-Legendre quadrature with $D=2$ and $N_q=7$.

!table id=tensor-npts caption=Resulting number of grid points for tensor grid
| $N_q$ | $D=2$ | $D=5$ | $D=8$ |
| - | - | - | - |
| 2 | 4 | 32 | 256 |
| 3 | 9 | 243 | 6,561 |
| 5 | 25 | 3,125 | 390,625 |
| 7 | 49 | 16,807 | 5,764,801 |

!media tensor_grid.svg id=tensor-grid

### Smolyak Sparse Grid

A Smolyak sparse grid is a multidimensional quadrature meant to integrate a complete monomial. This type of grid is effective for use with [PolynomialChaos.md]. The idea is that instead of taking the cartesian product of full order quadratures, it uses a combination of lower order quadratures to complete the monomial space. The Smolyak sparse grid scheme can reduce the number of points significantly in high dimensional space. [smolyak-npts] shows the number of points several different $D$ and $N_q$. [smolyak-grid] shows the points of a Smolyak grid of Gauss-Legendre quadrature with $D=2$ and $N_q=7$.

!table id=smolyak-npts caption=Resulting number of grid points for Smolyak sparse grid
| $N_q$ | $D=2$ | $D=5$ | $D=8$ |
| - | - | - | - |
| 2 | 5 | 11 | 17 |
| 3 | 14 | 66 | 153 |
| 5 | 55 | 1,001 | 4,845 |
| 7 | 140 | 7,997 | 74,613 |

!media smolyak_grid.svg id=smolyak-grid

### Clenshaw-Curtis Sparse Grid

A Clenshaw-Curtis (CC) grid is very similar to the Smolyak sparse, the only difference is that the Smolyak grid uses gauss quadrature, while (CC) grid appropriately uses (CC) quadrature. In one-dimension, Gauss quadrature is more accurate (with the same convergence rate) but CC has more nesting of points, which means that at high dimensions, CC grid might be more accurate with fewer number of sample points. [cc-npts] shows the number of points several different $D$ and $N_q$. [cc-grid] shows the points of a CC grid with $D=2$ and $N_q=7$

!table id=cc-npts caption=Resulting number of grid points for Clenshaw-Curtis sparse grid
| $N_q$ | $D=2$ | $D=5$ | $D=8$ |
| - | - | - | - |
| 2 | 5 | 11 | 17 |
| 3 | 9 | 51 | 129 |
| 5 | 21 | 301 | 1,937 |
| 7 | 49 | 1,113 | 11,937 |

!media cc_grid.svg id=cc-grid

## Implementation

The sampler uses inputted distributions to create a multidimensional quadrature of arbitrary order. The sampler can then be used to sample a sub-app. Another object can then use the results of the sampling and the quadrature weights from the sampler to integrate a quantity.

## Example Input File Syntax

First, distributions are made, which define the weighting function of the integration:

!listing poly_chaos/main_2d_quad.i block=Distributions

The QuadratureSampler then uses the distributions to create a quadrature with $N_q =$ `order`$+1$ points. The definition of `order` is important for use with polynomial chaos.

!listing poly_chaos/main_2d_quad.i block=Samplers/quadrature

!syntax parameters /Samplers/QuadratureSampler

!syntax inputs /Samplers/QuadratureSampler

!syntax children /Samplers/QuadratureSampler
