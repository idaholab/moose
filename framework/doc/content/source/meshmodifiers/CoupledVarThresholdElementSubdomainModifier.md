# CoupledVarThresholdElementSubdomainModifier

!syntax description /MeshModifiers/CoupledVarThresholdElementSubdomainModifier

## Overview

The `CoupledVarThresholdElementSubdomainModifier` changes the element subdomain if a coupled variable meets a particular criterion. The `threshold` and `criterion_type` are used to determine this criterion. By default, the element changes subdomain if the averaged value of the coupled variable in the element is `above` the `threshold`. Other types of criterion are `below` and `equal` to the `threshold` value.

The `CoupledVarThresholdElementSubdomainModifier` inherits from the [ElementSubdomainModifier.md]. Details on solution reinitialization, stateful material property reinitialization and moving boundary/interface nodeset/sideset modification can be found in the description of the [ElementSubdomainModifier.md].

## Irreversible Modification

Consider a unit square domain, and an auxiliary variable defined by the function $\phi(x,y,t) = (x-t)^2+y^2-0.5^2$. The function represents a signed-distance function of a circle of radius $0.5$ whose center is moving along the x-axis towards the right.

Initially, the domain is decomposed by a vertical line $x=0.25$. The elements on the left side of the vertical line have subdomain ID of 1, and the elements on the right side have subdomain ID of 2. The `CoupledVarThresholdElementSubdomainModifier` sets the coupled variable to be $\phi$, and the criterion to be `below` a `threshold` value of 0, so all the elements within the moving circle will change subdomain ID from 2 to 1:

!listing test/tests/meshmodifiers/element_subdomain_modifier/irreversible.i start=[moving_circle] end=[] include-end=true

!media large_media/mesh_modifiers/element_subdomain_modifier/irrev.png style=float:center;width:100%; caption=The result of a reversible element subdomain modifier at three different time steps

## Reversible Modification

The irreversible modification is useful for applications suhc as element death and activation, but the subdomain modificationan can be changed to reversible by setting the parameter `complement_subdomain_id` to 2. Then the subdomain ID of all elements outside the circle will be set to 2:

!listing test/tests/meshmodifiers/element_subdomain_modifier/reversible.i start=[moving_circle] end=[] include-end=true

!media large_media/mesh_modifiers/element_subdomain_modifier/rev.png style=float:center;width:100%; caption=The result of a reversible element subdomain modifier at three different time steps

!syntax parameters /MeshModifiers/CoupledVarThresholdElementSubdomainModifier

!syntax inputs /MeshModifiers/CoupledVarThresholdElementSubdomainModifier

!syntax children /MeshModifiers/CoupledVarThresholdElementSubdomainModifier
