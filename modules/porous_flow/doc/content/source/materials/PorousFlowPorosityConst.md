# PorousFlowPorosityConst

!syntax description /Materials/PorousFlowPorosityConst

A single value of porosity can be specified in the input file, or a spatially
varying porosity `AuxVariable` can be coupled to define a heterogeneous
porosity distribution.

Because the porosity does not change during the simulation, it is never clipped: an error is
generated if the porosity (or the `AuxVariable` anywhere in the mesh) is negative, or is less than
the optional `porosity_min`.

!syntax parameters /Materials/PorousFlowPorosityConst

!syntax inputs /Materials/PorousFlowPorosityConst

!syntax children /Materials/PorousFlowPorosityConst
