# Maxwellian

!syntax description /Distributions/Maxwellian

## Description

The Maxwellian (or Maxwell-Boltzmann) distribution object defines a
[Maxwell-Boltzmann distribution](https://en.wikipedia.org/wiki/Maxwell–Boltzmann_distribution) function with the provided `mass` and `temperature` parameters.
The probability density function (PDF) of the Maxwellian distribution is given by the [eq:maxwellian].

!equation id=eq:maxwellian
f(v \; | \; m, T) = \sqrt{ \frac{ m }{ 2 \pi k_B T }} \; \exp\left( - \frac{m v^2}{ 2 k_B T } \right)

where $v$ is a single component of a velocity vector, $m$ is the mass of the species in kg, and $T$ is the temperature the component of velocity is at in K

Since the single component Maxwellian is a special case of a normal distribution, where the mean $\mu = 0$, and the standard deviation is given by [eq:std],

!equation id=eq:std
\sigma = \sqrt{ \frac{ k_B T }{ m } }

this class is a specialization of [Normal.md].

## Example Input Syntax

The following input file defines a maxwellian distribution that is equivalent to a normal distribution with a mean of 0 and a standard deviation of 1.

!listing modules/stochastic_tools/test/tests/distributions/maxwellian.i block=Distributions

!syntax parameters /Distributions/Maxwellian

!syntax inputs /Distributions/Maxwellian

!syntax children /Distributions/Maxwellian

