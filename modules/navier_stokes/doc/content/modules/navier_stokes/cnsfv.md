# Compressible

## Method Notes

- Every quantity in the advection term for the mass continuity equation must be
  interpolated together in order to see a flat profile for the mass flux
  at steady state. And since I have seen instability with naive upwinding of
  vectors (indeed what is even the proper thing to do if the vector points in
  two different directions on either side of a face?), I believe the proper
  thing to do is to always use the mass-flux as the advector vector and to apply
  an average interpolation to it. This means that there will not be any form of
  upwinding for the advection term in the mass equation (e.g. the advected
  quantity is simply unity), but it's much more important to get mass continuity
  right then to potentially have more stability with a method (e.g. upwinding in
  the mass equation) that doesn't get mass continuity right. A user can observe
  improper mass continuity behavior if they take any channel-flow input which has a momentum
  source/sink, such that pressure and density change, and change the
  velocity/advected-quantity pairing such that the entire mass-flux isn't
  averaged together.
- All quantities should indeed be interpolated together for
  conservativeness. However, for sufficiently small time steps, even with an
  [ImplicitEuler.md] time integrator, you can observe oscillations when using an
  average/central-difference interpolation scheme.
- The reason that an all-upwind interpolation scheme is unstable/oscillatory is,
  per Mauricio Tano: "The issue with using just upwind is that you may
  overestimate the flux in the faces, leading to oscillations in the density
  that then propagate to the velocity field by continuity ... If you want to use
  upwind, you should bound the flux in the faces."
- The deficiencies in central-differencing and naive upwinding the advective
  term naturally lead to development/use of more complex schemes such as Godunov
  methods which solve exact or approximate Riemann problems at inter-cell
  boundaries. [Riemann problems](https://en.wikipedia.org/wiki/Riemann_problem)
  are specific initial value problems composed of a conservation equation
  together with piecewise constant initial data with a single discontinuity in
  the domain of interest. This continuity could be, for example, a front moving
  from an inlet boundary through a channel. Shocks and reafaction waves appear
  as characteristics in the solution of the Riemann problem, which is very
  useful for understanding equations like Euler.
