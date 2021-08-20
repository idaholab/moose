# INSFVMixingLengthReynoldsStress

This kernel uses a mixing-length model to compute the Reynolds stress,
$-\rho \overline{ \vec u' \vec u' }$, which appears in Reynolds-averaged
momentum equations. The velocity scaling is computed using Smagorinsky's
formulation.

!syntax parameters /FVKernels/INSFVMixingLengthReynoldsStress

!syntax inputs /FVKernels/INSFVMixingLengthReynoldsStress

!syntax children /FVKernels/INSFVMixingLengthReynoldsStress
