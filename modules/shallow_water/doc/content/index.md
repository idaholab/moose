## ShallowWater module

Finite-volume/DG implementation scaffolding for the 2D shallow-water equations (SWE).

Objects currently available:

- SWEFVFluxDGKernel: DG side flux assembly for [h, hu, hv].
- SWENumericalFluxHLL: HLL/Rusanov numerical flux (with approximate Jacobians).
- SWENumericalFluxHLLC: HLLC numerical flux with hydrostatic reconstruction.
- SWERDGReconstruction: Face-extrapolated values (stub pass-through).
- SWEBedSlopeSource: Bed slope source term for momentum.
- SWEFluxBC: Boundary flux BC wrapper for [h, hu, hv].
- SWEWallBoundaryFlux: Reflective wall boundary.
- SWEFreeOutflowBoundaryFlux: Physical flux F(U)·n outlet (basic).
- SWEOpenBoundaryRiemannFlux: Ghost-state Riemann open/outflow using HLLC/HLL.
- SlopeLimitingOneDSWE: 1D MUSCL slope limiter for [h, hu, hv] (minmod/MC/superbee).
