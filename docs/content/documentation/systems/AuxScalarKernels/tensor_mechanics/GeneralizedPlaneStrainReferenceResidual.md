# Generalized Plane Strain Reference Residual
!syntax description /AuxScalarKernels/GeneralizedPlaneStrainReferenceResidual

## Description
The AuxScalarKernel `GeneralizedPlaneStrainReferenceResidual` retrieves a reference residual value from [GeneralizedPlaneStrainUserObject](/systems/UserObjects/tensor_mechanics/GeneralizedPlaneStrainUserObject.md) for use with [ReferenceResidualProblem](/systems/Problem/contact/ReferenceResidualProblem.md).

## Example Input File Syntax
!listing modules/combined/test/tests/generalized_plane_strain_tm_contact/out_of_plane_pressure.i block=AuxScalarKernels/gps_ref_res

!syntax parameters /AuxScalarKernels/GeneralizedPlaneStrainReferenceResidual

!syntax inputs /AuxScalarKernels/GeneralizedPlaneStrainReferenceResidual

!syntax children /AuxScalarKernels/GeneralizedPlaneStrainReferenceResidual
