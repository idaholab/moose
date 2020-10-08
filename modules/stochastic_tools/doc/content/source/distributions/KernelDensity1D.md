# Kernel density estimation in 1D

!syntax description /Distributions/KernelDensity1D

## Description

The kernel density distribution in 1D object defines a [kernel density](https://en.wikipedia.org/wiki/Kernel_density_estimation) with the provided data vector $\mathbf{x}$, the kernel function $K(.)$,
 and the bandwidth $h$. The probability density of a kernel density is given by:

\begin{equation}
\label{eqn:kernel1d}
\hat{f}_h(x) = \frac{1}{nh} \sum_{i=1}^{n} K(\frac{x-x_i}{h})
\end{equation}

where $n$ is the number of data points in the data vector. Currently, the Gaussian and
the Uniform kernel functions are implemented. For the bandwidth, the user can either
provide a value or request an automatic computation using the data standard deviation or
the [Silverman's rule](https://en.wikipedia.org/wiki/Kernel_density_estimation).

## Example Input Syntax

The following input file defines a kernel density distribution with `kernel_density_1D_input.csv` as
 an example data input, `Gaussian` as the kernel function, and `Silverman's rule` for the bandwidth.
The data in `kernel_density_1D_input.csv` were drawn from a Normal distribution with a mean and
standard deviation of unity.

!listing modules/stochastic_tools/test/tests/distributions/kernel_density_1d_gaussian.i block=Distributions

!syntax parameters /Distributions/KernelDensity1D

!syntax inputs /Distributions/KernelDensity1D

!syntax children /Distributions/KernelDensity1D
