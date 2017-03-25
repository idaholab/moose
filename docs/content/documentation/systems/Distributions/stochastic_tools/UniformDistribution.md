# UniformDistribution

##Description
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
!input modules/stochastic_tools/tests/distributions/distribution_pps/distribution_pps.i block=Distributions

!parameters /Distributions/UniformDistribution

!inputfiles /Distributions/UniformDistribution

!childobjects /Distributions/UniformDistribution


