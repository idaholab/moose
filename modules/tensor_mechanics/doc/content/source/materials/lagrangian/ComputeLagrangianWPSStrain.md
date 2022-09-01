# ComputeLagrangianWPSStrain

!syntax description /Materials/ComputeLagrangianWPSStrain

## Overview

The `ComputeLagrangianWPSStrain` inherits from [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md). Only two displacement variables are coupled to compute the in-plane strain components. The out-of-plane strain is provided by another nonlinear variable served as a Lagrange multiplier to weakly enforce the plane stress condition.

!syntax parameters /Materials/ComputeLagrangianWPSStrain

!syntax inputs /Materials/ComputeLagrangianWPSStrain

!syntax children /Materials/ComputeLagrangianWPSStrain
