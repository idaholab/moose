# PiecewiseByBlockFunctorMaterial ADPiecewiseByBlockFunctorMaterial

!syntax description /FunctorMaterials/PiecewiseByBlockFunctorMaterial

## Overview

This object is useful for providing a material property value that is discontinuous from
subdomain to subdomain. [!param](/FunctorMaterials/PiecewiseByBlockFunctorMaterial/prop_name) is
required to specify the name of the material property. The map parameter
[!param](/FunctorMaterials/PiecewiseByBlockFunctorMaterial/subdomain_to_prop_value)
is used for specifying the property value on a subdomain name basis; the first member of each pair should
be a subdomain name while the second member should be a functor.

This material is a shorthand for specifying [GenericFunctorMaterial.md] restricted
to each block.

!alert note
ADPiecewiseByBlockFunctorMaterial is the version of this object with automatic differentiation
AD functors must be specified as the values on each block.

!syntax parameters /FunctorMaterials/PiecewiseByBlockFunctorMaterial

!syntax inputs /FunctorMaterials/PiecewiseByBlockFunctorMaterial

!syntax children /FunctorMaterials/PiecewiseByBlockFunctorMaterial
