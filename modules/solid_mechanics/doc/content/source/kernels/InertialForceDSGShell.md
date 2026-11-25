# InertialForceDSGShell

!syntax description /Kernels/InertialForceDSGShell

## Description

This kernel computes the inertial force of a DSG shell element on one of its six degrees of freedom, with mass-proportional Rayleigh damping and the damping term of the HHT time integration scheme, as [InertialForce](/InertialForce.md) does for continuum elements. The formulation is given in the inertia section of [DSGShellElements](/DSGShellElements.md).

- On the three displacements the mass is the consistent mass of the shell mid-surface, the density times the thickness integrated with the shape functions. It is the mass that [InertialForce](/InertialForce.md) would give with the mass per unit area as its density.
- On the rotations the kernel applies the rotary inertia $\rho t^3 / 12$ of the thickness to the two rotations in the plane of the element, through the projector $\mathbf{I} - \mathbf{e}_3 \mathbf{e}_3^T$ built from the element normal. The drilling rotation carries no inertia. The parameter `rotary_inertia` turns the rotary inertia off, in which case the kernels on the rotations contribute nothing.

The density is a material property (`density` by default) and the thickness comes from the material property `dsg_shell_thickness` of [ComputeDSGShellSectionStiffness](/ComputeDSGShellSectionStiffness.md). The velocities and accelerations are the ones of the time integrator, which must provide the second time derivative of the solution: use the [NewmarkBeta](/NewmarkBeta.md) time integrator.

One kernel is needed for each of the six degrees of freedom; `component` selects it as for [StressDivergenceDSGShell](/StressDivergenceDSGShell.md).

### Choosing the parameters

- `component`, `displacements` and `rotations` are as for [StressDivergenceDSGShell](/StressDivergenceDSGShell.md): six kernels, one per variable.
- `density` names the material property with the mass per unit volume of the shell material; the mass per unit area is the density times the thickness of the section stiffness material. A constant density is declared with [GenericConstantMaterial](/GenericConstantMaterial.md).
- `rotary_inertia` (default true) gives the two rotations in the plane of each element the rotary inertia $\rho t^3 / 12$ per unit area. Turning it off keeps the translational mass only and raises the frequencies slightly, 0.1 percent on the first mode of a plate whose thickness is a fifteenth of its length. The drilling rotation carries no inertia either way.
- `eta` is the mass-proportional Rayleigh damping coefficient, in units of one over time: the damping force is $\eta \mathbf{M} \dot{\mathbf{u}}$. Zero by default.
- `alpha` is the HHT parameter, between $-1/3$ and 0, which shifts the damping force toward the previous step, $\eta \mathbf{M} ((1 + \alpha) \dot{\mathbf{u}} - \alpha \dot{\mathbf{u}}_{\mathrm{old}})$. Zero by default, the plain Newmark scheme; use the same value as in the time integrator.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/dynamics/free_vibration.i block=Kernels

!syntax parameters /Kernels/InertialForceDSGShell

!syntax inputs /Kernels/InertialForceDSGShell

!syntax children /Kernels/InertialForceDSGShell
