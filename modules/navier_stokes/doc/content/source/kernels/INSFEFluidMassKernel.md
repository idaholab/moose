# INSFEFluidMassKernel

!syntax description /Kernels/INSFEFluidMassKernel

This kernel supports both regular and porous media flow, with the porosity specified
as a material property using the [!param](/Kernels/INSFEFluidMassKernel/porosity) parameter.

The mass equation is computed with the following residual for incompressible flow:

!equation
- \rho \vec{v} \nabla \psi

This kernel computes the PSPG stabilization terms from the momentum equation:

- the transient term $\rho \dot{\vec{v}} \psi_{pspg}$
- the convection term $\rho \ \epsilon \vec{v} * \nabla \vec{v} \psi_{pspg}$
- the pressure term $\epsilon \nabla p \psi_{pspg}$
- the gravity term $-\epsilon \rho \vec{g} \psi_{pspg}$
- a viscous term for regular media flow
- a viscous and a friction term for porous media flow


with $\psi_{pspg} = \tau_c \nabla \psi$, $\rho$ the density, $\vec{v}$ the superficial velocity,
$\vec{g}$ the gravity vector, $\epsilon$ the porosity, $p$ the pressure, $\tau_c$ a stabilization term computed by
the [INSFEMaterial.md], $\psi$ the test functions.

!syntax parameters /Kernels/INSFEFluidMassKernel

!syntax inputs /Kernels/INSFEFluidMassKernel

!syntax children /Kernels/INSFEFluidMassKernel
