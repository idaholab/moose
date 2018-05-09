# GeneralizedPlaneStrainAction

!syntax description /Modules/TensorMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction

This action simplifies the input file syntax for creating a generalized plane strain model. It creates following MOOSE objects (i.e. Kernel, ScalarKernel and UserObject) related to the out-of-plane scalar variable.

### Kernel

- [Generalized Plane Strain Off-diagonal Kernel](/GeneralizedPlaneStrainOffDiag.md): in-plane displacement variables and scalar out-of-plane strain coupling

### ScalarKernel

- [Generalized Plane Strain ScalarKernel](/GeneralizedPlaneStrain.md): out-of-plane equilibrium condition

### UserObject

- [Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObject.md): residual and diagonal Jacobian calculation for scalar out-of-plane strain variable

!syntax parameters /Modules/TensorMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction
