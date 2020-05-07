# Weibull

!syntax description /Distributions/Weibull

## Description

The Weibull object defines a translated Weibull distribution which
has a probability density function (PDF) defined as

!equation
f(x;\lambda,k,\theta) =
\begin{cases}
\frac{k}{\lambda}\left(\frac{x-\theta}{\lambda}\right)^{k-1}e^{-(\frac{x-\theta}{\lambda})^{k}} & x\geq0 ,\\
0 & x<0,
\end{cases}

where $k > 0$ and defines the shape parameter, $\lambda > 0$ and defines the scale parameter and $\theta$ is the location parameter of the distribution.

## Example Input Syntax

!listing modules/stochastic_tools/test/tests/distributions/weibull.i block=Distributions

!syntax parameters /Distributions/Weibull

!syntax inputs /Distributions/Weibull

!syntax children /Distributions/Weibull
