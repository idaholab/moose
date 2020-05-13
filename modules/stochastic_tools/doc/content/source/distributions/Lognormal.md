# Lognormal

## Description

The lognormal distribution object defines a
[lognormal distribution](https://en.wikipedia.org/wiki/Log-normal_distribution) function with the
provided `location` and `scale` parameters. The location parameter is equal to the median of the
underlying normal distribution (equal to ${\ln}\theta$, where $\theta$ is the median of the lognormal
distribution) and the scale parameter is equal to the standard deviation of the underlying normal
distribution. The probability density function (PDF) of the lognormal distribution is given by the
[eq:lognormal].

!equation id=eq:lognormal
f(x \; | \; m, s) =
\begin{cases}
\frac{1}{xs\sqrt{2\pi}}e^{\frac{-\left( {\ln} x-m \right)^2}{2s^2}} & x>0 ,\\
0 & x\leq0,
\end{cases}

where $m$ is the location parameter and $s$ is the scale parameter ($s > 0$).

## Example Input Syntax

The following input file defines a lognormal distribution with the location parameter -0.371 and the
scale parameter 0.52.

!listing modules/stochastic_tools/test/tests/distributions/lognormal.i block=Distributions

!syntax parameters /Distributions/Lognormal

!syntax inputs /Distributions/Lognormal

!syntax children /Distributions/Lognormal
