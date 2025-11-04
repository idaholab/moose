# INSFEFluidMomentumKernel

!syntax description /Kernels/INSFEFluidMomentumKernel

This kernel supports both regular and porous media flow, with the porosity specified
as a material property using the [!param](/Kernels/INSFEFluidMassKernel/porosity) parameter.

This kernel computes the following terms in the momentum equation:

- the convection term $\rho \ \epsilon \vec{v} * \nabla \vec{v} \psi$ (non-conservative form)
- the pressure term $\epsilon \nabla p \psi$ (volumetric form)
- the gravity term $-\epsilon \rho \vec{g} \psi$
- a viscous term for regular media flow
- a viscous and a friction term for porous media flow


!alert note
The pressure term can also be computed with an integration by parts. If so, the
boundary conditions for the momentum equations should also use an integration by parts.

!alert note
The convection term can also be computed using a conservative form. If so, other kernels
such as the [PINSFEFluidVelocityTimeDerivative.md] must also use a conservative form.


This kernel computes the SUPG stabilization terms for the momentum equation:

- the transient term $\rho \dot{\vec{v}} \psi_{supg}$
- the convection term $\rho \ \epsilon \vec{v} * \nabla \vec{v} \psi_{supg}$
- the pressure term $\epsilon \nabla p \psi_{supg}$
- the gravity term $-\epsilon \rho \vec{g} \psi_{supg}$
- a viscous term for regular media flow
- a viscous and a friction term for porous media flow


!syntax parameters /Kernels/INSFEFluidMomentumKernel

!syntax inputs /Kernels/INSFEFluidMomentumKernel

!syntax children /Kernels/INSFEFluidMomentumKernel
