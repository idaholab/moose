# ACBarrierFunction

The ACBarrierFunction kernel implements the term $\frac{\partial m}{\partial \eta}f(\eta)$
term for the free energy given in [!cite](moelans_quantitative_2008).
$m$ is the barrier energy coefficient.
In many phase field models, $m$ is constant, but in the case where it is a function
of one or more of the variables, this kernel should be included.

## Description

### General Information

This kernel should only be used for variables that are used in calculating $m$.
Other variables will do a lot of math to return a value of 0.

$m$ and its derivatives should be calculated in a materials block that uses the
DerivativeMaterialInterface, such as a DerivativeParsedMaterial.

The kernel only supports a single value of $\gamma$.
It should not be necessary to use multiple $\gamma$'s in models with a non-constant
$m$.

!syntax parameters /Kernels/ACBarrierFunction

!syntax inputs /Kernels/ACBarrierFunction

!syntax children /Kernels/ACBarrierFunction
