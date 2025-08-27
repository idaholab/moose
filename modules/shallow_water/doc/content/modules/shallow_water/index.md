## ShallowWater module

Finite-volume/DG implementation scaffolding for the 2D shallow-water equations (SWE).

Objects currently available:

- [SWEFVFluxDGKernel](SWEFVFluxDGKernel.md): DG side flux assembly for (h, hu, hv).
- [SWENumericalFluxHLL](SWENumericalFluxHLL.md): HLL/Rusanov numerical flux (with approximate Jacobians).
- [SWENumericalFluxHLLC](SWENumericalFluxHLLC.md): HLLC numerical flux with hydrostatic reconstruction.
- [SWERDGReconstruction](SWERDGReconstruction.md): Face-extrapolated values (stub pass-through).
- [SWEBedSlopeSource](SWEBedSlopeSource.md): Bed slope source term for momentum.
- [SWEFluxBC](SWEFluxBC.md): Boundary flux BC wrapper for (h, hu, hv).
- [SWEWallBoundaryFlux](SWEWallBoundaryFlux.md): Reflective wall boundary.
- [SWEFreeOutflowBoundaryFlux](SWEFreeOutflowBoundaryFlux.md): Physical flux F(U)·n outlet (basic).
- [SWEOpenBoundaryRiemannFlux](SWEOpenBoundaryRiemannFlux.md): Ghost-state Riemann open/outflow using HLLC/HLL.
- [SlopeLimitingOneDSWE](SlopeLimitingOneDSWE.md): 1D MUSCL slope limiter for (h, hu, hv) (minmod/MC/superbee).
