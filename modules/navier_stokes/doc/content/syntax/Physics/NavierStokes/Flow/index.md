# Navier Stokes Flow Physics syntax

This syntax was created for the [WCNSFVFlowPhysics.md] `Physics`.
The additional nesting is intended to allow the definition of multiple instances of this `Physics`,
with different block restrictions.

!syntax list /Modules/NavierStokesFV objects=False actions=True subsystems=False

!syntax list /Modules/NavierStokesFV objects=True actions=False subsystems=False

!syntax list /Modules/NavierStokesFV objects=False actions=False subsystems=True
