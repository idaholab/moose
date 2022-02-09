# PINSFVRhieChowInterpolator

!syntax description /UserObjects/PINSFVRhieChowInterpolator

## Overview

This object inherits from [INSFVRhieChowInterpolator.md] and does all the same
operations, but in addition optionally performs successive
interpolation-reconstruction operations on the porosity, which in effect is a
smoothing operation, helping to create monotone behavior near porosity
discontinuities. These interpolation-reconstruction operations are triggered by
setting the `smoothing_layers` parameter. One reconstruction corresponds to one
interpolation to the face and reconstruction back to the center, effectively
increasing the "stencil" of the porosity by one layer per `smoothing_layer`. If
performing smoothing_layers on the porosity, the input porosity functor cannot
be a MOOSE variable or a function of a MOOSE variable, as `smoothing_layers`
would then require algebraic ghosting of the solution vectors.

!syntax parameters /UserObjects/PINSFVRhieChowInterpolator

!syntax inputs /UserObjects/PINSFVRhieChowInterpolator

!syntax children /UserObjects/PINSFVRhieChowInterpolator
