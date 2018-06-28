# Generalized Plane Strain Action

## Description

This action simplifies the input file syntax for creating a generalized plane strain model. It creates the following MOOSE objects (i.e. Kernel, ScalarKernel and UserObject) related to the out-of-plane scalar variable.

### Kernel

- [Generalized Plane Strain Off-diagonal Kernel](/GeneralizedPlaneStrainOffDiag.md): in-plane displacement variables and scalar out-of-plane strain coupling

### ScalarKernel

- [Generalized Plane Strain ScalarKernel](/GeneralizedPlaneStrain.md): out-of-plane equilibrium condition

### UserObject

- [Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObject.md): residual and diagonal Jacobian calculation for scalar out-of-plane strain variable

## Generalized Plane Strain and Reference Residual

Generalized plane strain problems may use `ReferenceResidualProblem`. In this case, a reference scalar variable is needed to correspond with the scalar strain variable.

!listing modules/combined/test/tests/generalized_plane_strain_tm_contact/out_of_plane_pressure.i block=Problem

The reference scalar variable is set using the `AuxScalarKernel` [Generalized Plane Strain Reference Residual](/GeneralizedPlaneStrainReferenceResidual.md) using the [Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObject.md).

!listing modules/combined/test/tests/generalized_plane_strain_tm_contact/out_of_plane_pressure.i block=AuxScalarKernels


!syntax parameters /Modules/TensorMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction
