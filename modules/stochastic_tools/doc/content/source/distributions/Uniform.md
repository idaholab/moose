# Uniform

!syntax description /Distributions/Uniform

## Description

The uniform distribution is a probability distribution that has constant probability.
This is a continuous uniform distribution with the probability density function:

if $a < x < b$, then
!equation
f(x) = 1/(b - a)

if $x > b$ or $x < a$, then
!equation
f(x) = 0

where $a$ and $b$ are the lower bound and upper bound for the uniform distribution, respectively.

## Example Input Syntax

!listing modules/stochastic_tools/test/tests/distributions/uniform.i block=Distributions

!syntax parameters /Distributions/Uniform

!syntax inputs /Distributions/Uniform

!syntax children /Distributions/Uniform
