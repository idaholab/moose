# INSFVVelocityVariable

`INSFVVelocityVariable` is a finite volume variable. It overrides the default
computation of cell gradients because it must account for a modified computation
of boundary face values along a fully developed flow boundary. In addition it
toggles the `two_term_boundary_expansion` to `true` by default, which is the
parameter that determines whether extrapolated boundary face values are
determined from both the boundary cell centroid value and boundary cell centroid
gradient or just the boundary cell centroid value.

!syntax parameters /Variables/INSFVVelocityVariable

!syntax inputs /Variables/INSFVVelocityVariable

!syntax children /Variables/INSFVVelocityVariable
