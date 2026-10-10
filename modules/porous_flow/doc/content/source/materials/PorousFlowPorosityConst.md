# PorousFlowPorosityConst

!syntax description /Materials/PorousFlowPorosityConst

A single value of porosity can be specified in the input file, or a spatially
varying porosity `AuxVariable` can be coupled to define a heterogeneous
porosity distribution.

If the porosity (or the `AuxVariable` anywhere in the mesh) is less than `porosity_min`, which
is zero by default, the porosity is set to `porosity_min` there.

!syntax parameters /Materials/PorousFlowPorosityConst

!syntax inputs /Materials/PorousFlowPorosityConst

!syntax children /Materials/PorousFlowPorosityConst
