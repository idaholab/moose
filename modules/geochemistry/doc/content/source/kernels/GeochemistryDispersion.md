# GeochemistryDispersion

This Kernel implements the PDE fragment
\begin{equation}
\nabla(\phi D\nabla c) \ ,
\end{equation}
which is part of the [transport](theory/index.md) equations.  Here

- $\phi$ is the porosity, which may be a fixed real number or may be an `AuxVariable` that is spatially-dependent
- $D$ is the hydrodynamic dispersion tensor (called `tensor_coeff` in the input file)
- $c$ is the concentration (mol/m$^{3}$(aqueous solution)) of an aqueous species
- $\nabla$ denotes the vector of spatial derivatives

!syntax parameters /Kernels/GeochemistryDispersion

!syntax inputs /Kernels/GeochemistryDispersion

!syntax children /Kernels/GeochemistryDispersion

