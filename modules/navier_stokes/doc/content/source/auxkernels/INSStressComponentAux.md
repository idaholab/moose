# INSStressComponentAux

`INSStressComponentAux` calculates the stress (component) for an incompressible
Navier-Stokes simulation. Specify the `comp` param (allowed values 0, 1, 2) to
obtain the desired stress component. Note that the `velocity` parameter expects
a non-vector variable, e.g. this `AuxKernel` only currently makes sense with the
hand-coded set of INS objects.

!syntax description /AuxKernels/INSStressComponentAux

!syntax parameters /AuxKernels/INSStressComponentAux

!syntax inputs /AuxKernels/INSStressComponentAux

!syntax children /AuxKernels/INSStressComponentAux
