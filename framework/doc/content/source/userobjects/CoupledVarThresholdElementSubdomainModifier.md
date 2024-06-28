# CoupledVarThresholdElementSubdomainModifier

!syntax description /UserObjects/CoupledVarThresholdElementSubdomainModifier

## Overview

The `CoupledVarThresholdElementSubdomainModifier` can model

- Element death (with applications in ablation, fracture, etc.);
- Element activation (with applications in additive manufacturing, sintering, solidification, etc.);
- Moving interface (with applications in metal oxidation, phase transformation, melt pool, etc.).

The `CoupledVarThresholdElementSubdomainModifier` changes the element subdomain based on the given criterion. It also handles the corresponding

- Moving boundary/interface nodeset/sideset modification,
- Solution reinitialization, and
- Stateful material property reinitialization,

all of which are demonstrated below.

Consider a unit square domain, and an auxiliary variable defined by the function $\phi(x,y,t) = (x-t)^2+y^2-0.5^2$. The function represents a signed-distance function of a circle of radius $0.5$ whose center is moving along the x-axis towards the right.

Initially, the domain is decomposed by a vertical line $x=0.25$. The elements on the left side of the vertical line have subdomain ID of 1, and the elements on the right side have subdomain ID of 2. The `CoupledVarThresholdElementSubdomainModifier` is used to change the subdomain ID from 2 to 1 for elements within the circle.

## Reversible vs. irreversible modification

If the `CoupledVarThresholdElementSubdomainModifier` is applied onto the entire domain, and the parameter `complement_subdomain_id` is set to 2, then the subdomain ID of all elements outside the circle will be set to 2:

!listing test/tests/userobjects/element_subdomain_modifier/reversible.i start=[moving_circle] end=[] include-end=true

!media media/userobjects/rev.png style=float:center;width:100%; caption=The result of a reversible element subdomain modifier at three different time steps

However, in many applications, e.g. element death and activation, the equivalent movement of element subdomains is not reversible. In this case, omitting the parameter `complement_subdomain_id` will make the subdomain modification irreversible:

!listing test/tests/userobjects/element_subdomain_modifier/irreversible.i start=[moving_circle] end=[] include-end=true

!media media/userobjects/irrev.png style=float:center;width:100%; caption=The result of an irreversible element subdomain modifier at three different time steps

## Moving boundary/interface nodeset/sideset modification

The change of element subdomains will alter the definitions of certain sidesets and nodesets. The parameters `moving_boundaries` and `moving_boundary_subdomain_pairs` can optionally be used to modify the corresponding sidesets/nodesets. The pair of subdomains that each boundary in `moving_boundaries` lies between must be specified in the corresponding `moving_boundary_subdomain_pairs`, with the element side from the first subdomain added to the boundary. 

If the boundaries provided through `moving_boundaries` already exist, the modifier will attempt to modify the provided sidesets/nodesets whenever an element changes subdomain ID. If the boundaries do not exist, the modifier will create sidesets and nodesets with the provided names. 

!listing test/tests/userobjects/element_subdomain_modifier/moving_boundary.i start=[moving_circle] end=[] include-end=true

!media media/userobjects/nodeset.png style=float:center;width:100%; caption=The evolving nodeset (green) between subdomains 1 and 2, as created by the modifier without an existing boundary.

If only one boundary is provided but multiple pairs of subdomains are specified, then all the pairs are applied to the one boundary. Element sides on a subdomain's external boundary can be added by specifying only one subdomain.

!listing test/tests/userobjects/element_subdomain_modifier/external_moving_boundary.i start=[ext] end=[AuxVariables] include-end=false

!media media/userobjects/ext.png style=float:center;width:100%; caption=The evolving sideset (green) around subdomain 1, including the external element sides, from an existing boundary.

Nodal and integrated BCs can be applied on the moving boundary.

## Solution reinitialization

By default, all elements that change subdomain ID are reinitialized to the new subdomain's initial condition. Suppose the auxiliary variable $u$ has an initial variable value of $1$ in subdomain 1 and $-0.5$ in subdomain 2, and the variable value doubles at each timestep in subdomain 1:

!listing test/tests/userobjects/element_subdomain_modifier/initial_condition.i start=[ICs] end=[Postprocessors]

!media media/userobjects/init_cond.png style=float:center;width:100%; caption=The auxiliary variable $u$ is reinitialized to $1$, which doubles over the timestep to $2$, for all the elements that change subdomain ID to 1

## Stateful material property reinitialization

Similarly, all stateful material properties will be re-initialized when an element changes subdomain ID. Suppose initially the diffusivity is $0.5$ in subdomain 1 and $-1$ in subdomain 2, and the diffusivity doubles at each time step in subdomain 1:

!listing test/tests/userobjects/element_subdomain_modifier/stateful_property.i start=[Materials] end=[Executioner]

!media media/userobjects/stateful_prop.png style=float:center;width:100%; caption=The diffusivity is reinitialized to $0.5$, which doubles over the timestep to $1$, for all the elements that change subdomain ID to 1.

## Reinitialization restrictions

Depending on the physics, one may or may not want to reinitialize the solution when an element and its related nodes change subdomain ID. For the below examples, consider a unit square domain decomposed by vertical lines $x=0.3$ and $x=0.6$. The elements on the left have subdomain ID of 1, the elements in the middle have subdomain ID of 2, and the elements on the right have subdomain ID of 3. 

Two auxiliary variables are defined by the functions $\phi(x,y,t) = (x-t)^2+y^2-0.3^2$ and $\phi(x,y,t) = (x-t)^2+(y-1)^2-0.3^2$, which represent the signed-distance functions of circles of radius $0.3$ whose centers are moving along the bottom and top of the square respectively. The `CoupledVarThresholdElementSubdomainModifier` is used to change the subdomain ID to 1 for elements within the bottom circle, and to subdomain ID 2 for elements within the top circle.

An auxiliary variable $u$ is defined over the domain, with initial values of 1, 2, and 3 in subdomains 1, 2, and 3 respectively:

!listing test/tests/userobjects/element_subdomain_modifier/reinitialization.i start=[UserObjects] end=[AuxVariables]

!media media/userobjects/orig.png style=float:center;width:100%; caption=The default behaviour of the modifier is to reinitializate all elements that change subdomain ID.

However, if a list of subdomains (IDs or names) is provided through the parameter `reinitialize_subdomains`, the reinitialization only occurs if an element changes subdomain ID, and the new subdomain ID is in the list:

!listing test/tests/userobjects/element_subdomain_modifier/reinitialization_into.i start=[UserObjects] end=[AuxVariables]

!media media/userobjects/into.png style=float:center;width:100%; caption=Reinitialization of only the elements that change subdomain ID to 1.

If an empty list is given in `reinitialize_subdomains`, then there is no reinitialization of any elements that change subdomain ID.

!listing test/tests/userobjects/element_subdomain_modifier/no_reinitialization.i start=[UserObjects] end=[AuxVariables]

!media media/userobjects/none.png style=float:center;width:100%; caption=No reinitialization of any elements that change subdomain ID.

Reinitialization can be further restricted by setting the parameter `previous_subdomain_reinitialized` to `false`. The modifier will then additionally check the changing element's previous subdomain ID. Reinitialization will only occur if the previous subdomain ID was not in the list provided in the parameter `reinitialize_subdomains`.

!listing test/tests/userobjects/element_subdomain_modifier/reinitialization_from_into.i start=[UserObjects] end=[AuxVariables]

!media media/userobjects/from_into.png style=float:center;width:100%; caption=Reinitialization of only the elements which change subdomain ID from 3, to subdomain IDs 1 or 2.

!syntax parameters /UserObjects/CoupledVarThresholdElementSubdomainModifier

!syntax inputs /UserObjects/CoupledVarThresholdElementSubdomainModifier

!syntax children /UserObjects/CoupledVarThresholdElementSubdomainModifier
