# PINSFVRhieChowInterpolator

!syntax description /UserObjects/PINSFVRhieChowInterpolator

## Overview

This object inherits from [INSFVRhieChowInterpolator.md] and does all the same
operations, but in addition optionally performs successive
interpolation-reconstruction operations on the porosity, which in effect is a
smoothing operation, helping to create monotone behavior near porosity
discontinuities. These interpolation-reconstruction operations are triggered by
setting the `reconstructions` parameter. One reconstruction corresponds to one
interpolation to the face and reconstruction back to the center, effectively
increasing the "stencil" of the porosity by one layer per `reconstruction`. If
performing reconstructions on the porosity, the input porosity functor cannot be
a MOOSE variable as reconstructions would then require algebraic ghosting of the
solution vectors.

!syntax parameters /UserObjects/PINSFVRhieChowInterpolator

!syntax inputs /UserObjects/PINSFVRhieChowInterpolator

!syntax children /UserObjects/PINSFVRhieChowInterpolator
