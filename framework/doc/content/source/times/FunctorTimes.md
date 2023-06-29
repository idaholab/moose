# FunctorTimes

!syntax description /Times/FunctorTimes

The functor and its factor will be evaluated in the element containing the (0,0,0) point, the latter of which should lie
inside the mesh. They will be evaluated at the current simulation time, but only at the specified
`execute_on` flags.

The time computed will be appended to the times vector. The times vector is regularly sorted by default.

!syntax parameters /Times/FunctorTimes

!syntax inputs /Times/FunctorTimes

!syntax children /Times/FunctorTimes
