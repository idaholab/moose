# PorousFlowBrine

!syntax description /Materials/PorousFlowBrine

A class for providing brine fluid properties (water and NaCl) where the properties are computed using [BrineFluidProperties](/BrineFluidProperties.md).

The salt mass fraction (kg/kg) can be set as a nonlinear variable, an auxillary variable, or simply as a constant value using the `xnacl` input parameter. If no value is provided, the salinity is assumed to be zero and the brine properties will be just the water properties.

!alert note
If `xnacl` is a nonlinear variable, it is important to list that variable in the [PorousFlowDictator](/PorousFlowDictator.md) to ensure that the correct Jacobian contribution is calculated

This material also allows the user to provide a specific water fluid properties userobject. If no water userobject is provided, the [Water97FluidProperties](/Water97FluidProperties.md) fluid properties are used by default. This option allows users to implement a tabulated version of the water fluid properties to reduce the computational burden, see [TabulatedFluidProperties](/TabulatedFluidProperties.md) for details.

!syntax parameters /Materials/PorousFlowBrine

!syntax inputs /Materials/PorousFlowBrine

!syntax children /Materials/PorousFlowBrine
