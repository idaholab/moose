# BodyForceLM

!syntax description /Kernels/BodyForceLM

## Overview

This object is equivalent to [BodyForce.md] except it adds its residual both to
the primal equation on which the body force is being applied and to a Lagrange
Multiplier (LM) equation when an LM is present. We can use an LM for enforcing
non-negative concentrations, but its introduction generally leads to creation of
a saddle point problem. However, by adding primal equation components to the LM
equation, we create a consistent stabilization that removes the saddle
point. See [LMKernel.md] for more details.

!syntax parameters /Kernels/BodyForceLM

!syntax inputs /Kernels/BodyForceLM

!syntax children /Kernels/BodyForceLM
