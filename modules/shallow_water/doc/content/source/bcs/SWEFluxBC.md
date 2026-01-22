# SWEFluxBC

!syntax description /BCs/SWEFluxBC

!syntax parameters /BCs/SWEFluxBC

!syntax inputs /BCs/SWEFluxBC

!syntax children /BCs/SWEFluxBC

Note

- When using boundary flux user objects that expect hydrostatic reconstruction (e.g. the open
  outflow `SWEOpenBoundaryRiemannFlux`), you can pass a cell-constant bathymetry field via the
  optional `b_var` parameter. This appends `b` as a fourth entry to the conservative vector passed
  to the boundary flux and preserves lake-at-rest at boundaries with variable bathymetry.
