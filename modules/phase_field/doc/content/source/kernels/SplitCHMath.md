# SplitCHMath

!syntax description /Kernels/SplitCHMath

Implements the bulk free free energy density

!equation
f_\text{bulk}(c) = \frac14(1-c)^2(1+c)^2

where $c$ is the concentration variable the kernel is operating on. The minima
of this free energy density lie at $c=1$ and $c=-1$.

It is used together with [`SplitCHWRes`](/SplitCHWRes.md) and
[`CoupledTimeDerivative`](/CoupledTimeDerivative.md) to set up a system of two
first order PDEs using a concentration order parameter and a chemical potential
variable.

!syntax parameters /Kernels/SplitCHMath

!syntax inputs /Kernels/SplitCHMath

!syntax children /Kernels/SplitCHMath

!bibtex bibliography
