# ADWCNSFV2PSlipVelocityFunctorMaterial

This material computes the slip velocity between a dispersed phase and the continuous phase for the
two-phase mixture model, and optionally the diffusion velocity derived from it, with automatic
differentiation. It is the version of [WCNSFV2PSlipVelocityFunctorMaterial.md] for the nonlinear
finite volume discretization, and it accepts an [INSFVVelocityVariable.md] or a
[MooseLinearVariableFV.md] velocity. Both are one templated implementation of the same closure; see
[WCNSFV2PSlipVelocityFunctorMaterial.md] for the closure, the drag laws and their references.

!syntax parameters /FunctorMaterials/ADWCNSFV2PSlipVelocityFunctorMaterial

!syntax inputs /FunctorMaterials/ADWCNSFV2PSlipVelocityFunctorMaterial

!syntax children /FunctorMaterials/ADWCNSFV2PSlipVelocityFunctorMaterial
