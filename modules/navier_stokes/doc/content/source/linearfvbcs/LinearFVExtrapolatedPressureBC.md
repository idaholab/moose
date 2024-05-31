# LinearFVExtrapolatedPressureBC

## Description

This boundary condition is created for pressure variables in the pressure Poisson
equation. It behaves just like a [LinearFVAdvectionDiffusionExtrapolatedBC.md] but
its response depends on the usage:

  - When it is used for the computation of gradients, the user can prescribe two-term extrapolation
    to the boundary faces to increase accuracy.
  - When it is used for building the pressure Poisson equation, it assumes a one-term expansion. The
    reason behind this is that for cases when the pressure needs to be pinned, adding boundary-related
    terms to the right hand side of the system will emulate a boundary source which yields unphysical results.


!syntax parameters /LinearFVBCs/LinearFVExtrapolatedPressureBC

!syntax inputs /LinearFVBCs/LinearFVExtrapolatedPressureBC

!syntax children /LinearFVBCs/LinearFVExtrapolatedPressureBC
