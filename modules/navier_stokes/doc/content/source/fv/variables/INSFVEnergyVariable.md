# INSFVEnergyVariable

`INSFVEnergyVariable` is a finite volume variable that
toggles the `two_term_boundary_expansion` to `true` by default. This is the
parameter that determines whether extrapolated boundary face values are
determined from both the boundary cell centroid value and boundary cell centroid
gradient or just the boundary cell centroid value.

!syntax parameters /Variables/INSFVEnergyVariable

!syntax inputs /Variables/INSFVEnergyVariable

!syntax children /Variables/INSFVEnergyVariable
