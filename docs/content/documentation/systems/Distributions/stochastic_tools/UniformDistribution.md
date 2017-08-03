# UniformDistribution

## Description
The uniform distribution is a probability distribution that has constant probability.
This is a continuous uniform distribution with the probability density function:

if $a < x < b$, then
\begin{equation}
f(x) = 1/(b - a)
\end{equation}
if $x > b$ or $x < a$, then
\begin{equation}
f(x) = 0
\end{equation}
where $a$ and $b$ are the lower bound and upper bound for the uniform distribution, respectively.

## Example Input Syntax

!listing modules/stochastic_tools/test/tests/distributions/uniform.i block=Distributions

!syntax parameters /Distributions/UniformDistribution

!syntax inputs /Distributions/UniformDistribution

!syntax children /Distributions/UniformDistribution
