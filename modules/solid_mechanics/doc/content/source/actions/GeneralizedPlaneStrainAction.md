# GeneralizedPlaneStrainAction

!syntax description /Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction

!template load file=modules/solid_mechanics/common/GeneralizedPlaneStrainActionOverview.md.template

## Generalized Plane Strain and Reference Residual

Generalized plane strain problems may use `ReferenceResidualProblem`. In this case, a reference scalar variable is needed to correspond with the scalar strain variable.

!listing modules/combined/test/tests/generalized_plane_strain_tm_contact/out_of_plane_pressure.i block=Problem

The reference scalar variable is set using the `AuxScalarKernel` [Generalized Plane Strain Reference Residual](/GeneralizedPlaneStrainReferenceResidual.md) using the [Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObject.md).

!listing modules/combined/test/tests/generalized_plane_strain_tm_contact/out_of_plane_pressure.i block=AuxScalarKernels

!syntax parameters /Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction
