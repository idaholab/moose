# GeochemistryTimeDerivative

This Kernel implements the PDE fragment
\begin{equation}
\phi \frac{\partial c}{\partial t} \ ,
\end{equation}
which is part of the [transport](theory/index.md) equations.  Here

- $\phi$ is the porosity, which may be a fixed real number or may be an `AuxVariable` that is spatially-dependent
- $c$ is the concentration (mol/m$^{3}$(aqueous solution)) of an aqueous species
- $t$ is time

Two notable features of this Kernel are:

- it should not be used for cases were porosity is time-dependent (in this case $\phi$ should be inside the time derivative);
- mass lumping to the nodes is used in order to reduce the possibility that concentration becomes negative.

!syntax parameters /Kernels/GeochemistryTimeDerivative

!syntax inputs /Kernels/GeochemistryTimeDerivative

!syntax children /Kernels/GeochemistryTimeDerivative

