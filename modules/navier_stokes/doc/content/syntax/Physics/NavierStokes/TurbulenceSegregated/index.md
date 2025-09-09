# Navier Stokes Turbulence Physics for Segregated Solves

This syntax was created for the [WCNSLinearFVTurbulencePhysics.md] `Physics`.
The additional nesting is intended to allow the definition of multiple instances of this `Physics`,
with different block restrictions.

!syntax list /Physics/NavierStokes/TurbulenceSegregated objects=True actions=False subsystems=False

!syntax list /Physics/NavierStokes/TurbulenceSegregated objects=False actions=False subsystems=True

!syntax list /Physics/NavierStokes/TurbulenceSegregated objects=False actions=True subsystems=False
