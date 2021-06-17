# Peridynamic Generalized Plane Strain Action System

## Description

This action sets up a generalized plane strain model, including kernel to provide coupled off-diagonal Jacobian entries, scalar kernel to provide residual and diagonal Jacobian, and user object to compute residual and diagonal Jacobian for scalar variable.

## Constructed MooseObjects

!table id=pd_gps_action_table caption=Correspondence Among Action Functionality and MooseObjects for the `GeneralizedPlaneStrainPD` Action
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Out-of-plane scalar variable equilibrium condition | [Generalized Plane Strain ScalarKernel](/GeneralizedPlaneStrainPD.md) | `generalized_plane_strain_uo`: UserObject name of the GeneralizedPlaneStrainUserObjectBasePD |
| Residual and diagonal Jacobian calculation for scalar out-of-plane strain variables | [Ordinary State-based Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObjectOSPD.md) or [Non-Ordinary State-based Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObjectNOSPD.md) | `out_of_plane_stress_variable`: Auxiliary variable name for out-of-plane stress for ordinary state-based generalized plane strain model; or `none` is required for non-ordinary state-based generalized plane strain model|
| Scalar out-of-plan strain coupling with in-plane field variables | [Ordinary State-based Generalized Plane Strain Off-diagonal Kernel](/GeneralizedPlaneStrainOffDiagOSPD.md) or [Non-Ordinary State-based Generalized Plane Strain Off-diagonal Kernel](/GeneralizedPlaneStrainOffDiagNOSPD.md) | `scalar_out_of_plane_strain`: a string of scalar variable for the out-of-plane strain direction; `displacements` : a string of the displacement field (in-plane) variables; `temperature`: a string of the temperature field variable |

## Example Input Syntax

### Subblocks

The subblocks of the GeneralizedPlaneStrain action are what triggers MOOSE objects to be built.
If none of the mechanics is subdomain restricted a single subblock will be used

!listing modules/peridynamics/test/tests/generalized_plane_strain/generalized_plane_strain_OSPD.i block=Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain

If different generalized plane strain models are needed, multiple subblocks with subdomain restrictions
can be used.

!listing modules/peridynamics/test/tests/generalized_plane_strain/generalized_plane_strain_squares_OSPD.i block=Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain

Parameters supplied at the `[Modules/Peridynamics/GeneralizedPlaneStrain]` level act as defaults for the GeneralizedPlaneStrain action subblocks.

!syntax parameters /Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainActionPD


## Associated Actions

!syntax list /Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain objects=True actions=False subsystems=False

!syntax list /Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain objects=False actions=False subsystems=True

!syntax list /Modules/Peridynamics/Mechanics/GeneralizedPlaneStrain objects=False actions=True subsystems=False
