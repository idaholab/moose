# PiecewiseByBlockFunctorMaterial

!syntax description /Materials/PiecewiseByBlockFunctorMaterial

## Overview

This object is useful for providing a material property value that is discontinuous from
subdomain to subdomain. [!param](/Materials/PiecewiseByBlockFunctorMaterial/prop_name) is
required to specify the name of the material property. The map parameter
[!param](/Materials/PiecewiseByBlockFunctorMaterial/subdomain_to_prop_value)
is used for specifying the property value on a subdomain name basis; the first member of each pair should
be a subdomain name while the second member should be a real value.

This material is a shorthand for specifying [GenericConstantMaterial.md] restricted
to each block.

!syntax parameters /Materials/PiecewiseByBlockFunctorMaterial

!syntax inputs /Materials/PiecewiseByBlockFunctorMaterial

!syntax children /Materials/PiecewiseByBlockFunctorMaterial
