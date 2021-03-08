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
