# QuadratureSampler

!syntax description /Samplers/QuadratureSampler

## Overview

Numerical quadrature is a method of selecting specific points and weights in integrate a function,

\begin{equation}
\int_{a}^{b}y(x)f(x)dx = \sum_{q=1}^{N_q}w_qy(x_q) ,
\end{equation}

where $y(x)$ is the function being integrated, $f(x)$ is weighting function, $N_q$ is the number of quadrature points, $w_q$ and $x_q$ are quadrature weights and points, respectively. The sampler generates the points and weights based on the weighting function and the desired integration order. A uniform weighting function uses Gauss-Legendre quadrature and a normal weighting function uses Gauss-Hermite quadrature. These two quadratures can exactly integrate a polynomial of order $2N_q-1$. Multidimensional quadratures are created by taking a Cartesian product of the points and a Kroenecker product of the weights. Although, sparse grids will be made available in the near future.

## Implementation

The sampler uses inputted distributions to create a multidimensional quadrature of arbitrary order. The sampler can then be used to sample a sub-app. Another object can then use the results of the sampling and the quadrature weights from the sampler to integrate a quantity.

## Example Input File Syntax

First, distributions are made, which define the weighting function of the integration:

!listing poly_chaos/master_2d_quad.i block=Distributions

The QuadratureSampler then uses the distributions to create a quadrature with $N_q =$ `order`$+1$ points. The definition of `order` is important for use with polynomial chaos.

!listing poly_chaos/master_2d_quad.i block=Samplers/quadrature

!syntax parameters /Samplers/QuadratureSampler

!syntax inputs /Samplers/QuadratureSampler

!syntax children /Samplers/QuadratureSampler
