# SoretDiffusion

!syntax description /Kernels/SoretDiffusion

Implements weak form

!equation
\left(\frac{D\cdot Q\cdot c}{k_B\cdot T^2}, \nabla T \cdot\nabla \psi\right),

where $c$ is the concentration variable the kernel is operating on, $Q$ the heat
of transport, and $D$ the diffusivity.

It is used together with [`SplitCHWRes`](/SplitCHWRes.md),
[`SplitCHParsed`](/SplitCHParsed.md), and
[`CoupledTimeDerivative`](/CoupledTimeDerivative.md) to set up a system of two
first order PDEs using a concentration order parameter and a chemical potential
variable.

!syntax parameters /Kernels/SoretDiffusion

!syntax inputs /Kernels/SoretDiffusion

!syntax children /Kernels/SoretDiffusion

!bibtex bibliography
