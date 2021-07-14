# FVPropValPerSubdomainMaterial

!syntax description /Materials/FVPropValPerSubdomainMaterial

## Overview

This object is useful for providing a material property value, for finite volume
calculations, that is discontinuous from subdomain to subdomain. `prop_name` is
required. The map parameter `subdomain_to_prop_value` is used for specifying the
property value on a subdomain name basis; the first member of each pair should
be a subdomain name while the second member should be a real value.

!syntax parameters /Materials/FVPropValPerSubdomainMaterial

!syntax inputs /Materials/FVPropValPerSubdomainMaterial

!syntax children /Materials/FVPropValPerSubdomainMaterial
