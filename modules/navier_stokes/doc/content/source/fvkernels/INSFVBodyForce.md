# INSFVBodyForce

This object implements the exact same functionality as [FVBodyForce.md] but it's
dedicated to use in the Navier-Stokes momentum equation because it feeds its
body force contribution into the [INSFVRhieChowInterpolator.md] object for
interpolation and reconstruction operations.

!syntax parameters /FVKernels/INSFVBodyForce

!syntax inputs /FVKernels/INSFVBodyForce

!syntax children /FVKernels/INSFVBodyForce
