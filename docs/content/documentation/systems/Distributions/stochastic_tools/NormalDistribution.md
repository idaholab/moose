# NormalDistribution

!!!warning
    The NormalDistribution object requires that libMesh be configured to utilize an external
    [Boost](www.boost.org) library. This may be done by using the `--with-boost` configuration option
    when compiling libMesh.

## Description
The normal (or Gaussian) distribution object defines a [normal distribution](https://en.wikipedia.org/wiki/Normal_distribution) function with the provided `mean` and `standard_deviation` parameters. The probability density function (PDF) of the normal distribution is given by the equation

\begin{equation}
f(x \; | \; \mu, \sigma^2) = \frac{1}{\sqrt{2\pi\sigma^2} } \; e^{ -\frac{(x-\mu)^2}{2\sigma^2} }
\end{equation}
where $\mu$ is the mean and $\sigma$ is the standard deviation ($\sigma > 0$) of the distribution.

## Example Input Syntax
The following input file defines a normal distribution with a mean of 0 and a standard deviation of 1.

!listing /modules/stochastic_tools/test/tests/distributions/normal.i block=Distributions

!syntax parameters /Distributions/NormalDistribution

!syntax inputs /Distributions/NormalDistribution

!syntax children /Distributions/NormalDistribution
