# Normal

!syntax description /Distributions/Normal

## Description

The normal (or Gaussian) distribution object defines a
[normal distribution](https://en.wikipedia.org/wiki/Normal_distribution) function with the provided
`mean` and `standard_deviation` parameters. The probability density function (PDF) of the normal
distribution is given by the [eq:normal].

!equation id=eq:normal
f(x \; | \; \mu, \sigma^2) = \frac{1}{\sqrt{2\pi\sigma^2} } \; e^{ -\frac{(x-\mu)^2}{2\sigma^2} }

where $\mu$ is the mean and $\sigma$ is the standard deviation ($\sigma > 0$) of the distribution.

This implementation of a normal distribution uses a numerical approximation described in
[!cite](kennedy2018statistical).

## Example Input Syntax

The following input file defines a normal distribution with a mean of 0 and a standard deviation of 1.

!listing modules/stochastic_tools/test/tests/distributions/normal.i block=Distributions

!syntax parameters /Distributions/Normal

!syntax inputs /Distributions/Normal

!syntax children /Distributions/Normal

!bibtex bibliography
