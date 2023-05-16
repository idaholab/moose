# TruncatedNormal

!syntax description /Distributions/TruncatedNormal

## Description

The truncated normal (or Gaussian) distribution object defines a
[normal distribution](https://en.wikipedia.org/wiki/Normal_distribution) function with the provided
`mean`, `standard_deviation` and truncation range (`lower_bound` and `upper_bound`) parameters. The probability density function (PDF) of the truncated normal
distribution is given by the [eq:truncated_normal].

!equation id=eq:truncated_normal
\psi(x \; | \; \mu, \sigma^2, a, b) = \begin{cases}
    \frac{\phi((x \; | \; \mu, \sigma^2)}{\Phi(x\;|\;\mu,\sigma^2,b)-\Phi(x\;|\;\mu,\sigma^2,a)},& \text{if } a<x<b\\
    0,              & \text{otherwise}
\end{cases}

The parameters $\mu$ and $\sigma$ are the mean and standard deviation of the general normal PDF. The $\phi((x \; | \; \mu, \sigma^2)$ and $\Phi((x \; | \; \mu, \sigma^2)$ are the general normal PDF and cumulative distribution function (CDF).

## Example Input Syntax

The following input file defines a normal distribution with a mean of 100, a standard deviation of 25, and truncation range (50,150).

!listing modules/stochastic_tools/test/tests/distributions/truncated_normal.i block=Distributions

!syntax parameters /Distributions/TruncatedNormal

!syntax inputs /Distributions/TruncatedNormal

!syntax children /Distributions/TruncatedNormal

!bibtex bibliography
