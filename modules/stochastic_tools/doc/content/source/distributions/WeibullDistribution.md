# WeibullDistribution

## Description

The WeibullDistribution object defines a translated Weibull distribution which
has a probability density function (PDF) defined as

\begin{equation}
f(x;\lambda,k,\theta) =
\begin{cases}
\frac{k}{\lambda}\left(\frac{x-\theta}{\lambda}\right)^{k-1}e^{-(\frac{x-\theta}{\lambda})^{k}} & x\geq0 ,\\
0 & x<0,
\end{cases}
\end{equation}
where $k > 0$ and defines the shape parameter, $\lambda > 0$ and defines the scale parameter and $\theta$ is the location parameter of the distribution.

## Example Input Syntax

!listing modules/stochastic_tools/test/tests/distributions/weibull.i block=Distributions

!syntax parameters /Distributions/WeibullDistribution

!syntax inputs /Distributions/WeibullDistribution

!syntax children /Distributions/WeibullDistribution
