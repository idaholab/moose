# PiecewiseConstantByBlockMaterial

!syntax description /Materials/PiecewiseConstantByBlockMaterial

## Overview

This object is useful for providing a material property value, for finite volume
calculations, that is discontinuous from subdomain to subdomain. `prop_name` is
required. The map parameter `subdomain_to_prop_value` is used for specifying the
property value on a subdomain name basis; the first member of each pair should
be a subdomain name while the second member should be a real value.

!syntax parameters /Materials/PiecewiseConstantByBlockMaterial

!syntax inputs /Materials/PiecewiseConstantByBlockMaterial

!syntax children /Materials/PiecewiseConstantByBlockMaterial
