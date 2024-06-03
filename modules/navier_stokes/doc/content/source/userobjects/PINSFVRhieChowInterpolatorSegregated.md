# PINSFVRhieChowInterpolatorSegregated

!syntax description /UserObjects/PINSFVRhieChowInterpolatorSegregated

## Overview

Similarly to [INSFVRhieChowInterpolatorSegregated.md], this object is responsible for generating the coupling fields between the momentum and pressure equations in [SIMPLENonlinearAssembly.md].
The main difference between this object and [INSFVRhieChowInterpolatorSegregated.md] is that
this object needs to account for the fact that in the porous medium equations there is a
porosity multiplier on the pressure gradient term in the momentum equation: $-\epsilon \nabla p$.

!syntax parameters /UserObjects/PINSFVRhieChowInterpolatorSegregated

!syntax inputs /UserObjects/PINSFVRhieChowInterpolatorSegregated

!syntax children /UserObjects/PINSFVRhieChowInterpolatorSegregated
