# INSFVBodyForce

This object implements both the functionalities as [FVBodyForce.md] and [FVCoupledForce.md] but it's
dedicated to use in the Navier-Stokes momentum equation.

A body force in the momentum equation can be a momentum source,
like a pump, or a momentum sink, like a volumetric friction term. Friction terms may also be modeled with [PINSFVMomentumFriction.md].

## Example input syntax

In this example, the `INSFVBodyForce` is used to apply a forcing function to the momentum equation in a method of manufactured solution study.

!listing test/tests/finite_volume/ins/mms/channel-flow/2d-rc.i block=FVKernels

!syntax parameters /FVKernels/INSFVBodyForce

!syntax inputs /FVKernels/INSFVBodyForce

!syntax children /FVKernels/INSFVBodyForce
