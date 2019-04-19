# AuxVariables System

The AuxVariables block within the input file may be used to create "auxiliary" variables that
act, with respect to the interaction with other objects, like "nonlinear" variables (see
[syntax/Variables/index.md]). Please refer to the [AuxKernels/index.md] for complete details
regarding the use of auxiliary variables.

!syntax list /AuxVariables objects=True actions=False subsystems=False

!syntax list /AuxVariables objects=False actions=False subsystems=True

!syntax list /AuxVariables objects=False actions=True subsystems=False
