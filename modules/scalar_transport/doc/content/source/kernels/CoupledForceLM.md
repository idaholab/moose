# CoupledForceLM

!syntax description /Kernels/CoupledForceLM

## Overview

This object is equivalent to [CoupledForce.md] except it adds its residual both to
the primal equation on which the coupled force is being applied and to a Lagrange
Multiplier (LM) equation when an LM is present. We can use an LM for enforcing
non-negative concentrations, but its introduction generally leads to creation of
a saddle point problem. However, by adding primal equation components to the LM
equation, we create a consistent stabilization that removes the saddle
point. See [LMKernel.md] for more details.

!syntax parameters /Kernels/CoupledForceLM

!syntax inputs /Kernels/CoupledForceLM

!syntax children /Kernels/CoupledForceLM
