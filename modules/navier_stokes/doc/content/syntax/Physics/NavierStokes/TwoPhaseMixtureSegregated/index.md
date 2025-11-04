# Navier Stokes Two Phase Mixture Physics using linear finite volume discretization and segregated solver

This syntax was created for the [WCNSLinearFVTwoPhaseMixturePhysics.md] `Physics`.
The additional nesting is intended to allow the definition of multiple instances of this `Physics`,
with different block restrictions.

!syntax list /Physics/NavierStokes/TwoPhaseMixtureSegregated objects=True actions=False subsystems=False

!syntax list /Physics/NavierStokes/TwoPhaseMixtureSegregated objects=False actions=False subsystems=True

!syntax list /Physics/NavierStokes/TwoPhaseMixtureSegregated objects=False actions=True subsystems=False
