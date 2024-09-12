# ElementSubdomainModifier

## Overview

The `ElementSubdomainModifier` modifies an element subdomain ID. This class is inherited by other mesh modifiers, such as [CoupledVarThresholdElementSubdomainModifier.md] and [TimedSubdomainModifier.md], which implement different criteria for a subdomain ID to be modified.

The `ElementSubdomainModifier` can model

- Element death (with applications in ablation, fracture, etc.);
- Element activation (with applications in additive manufacturing, sintering, solidification, etc.);
- Moving interface (with applications in metal oxidation, phase transformation, melt pool, etc.).

The `ElementSubdomainModifier` only changes the element's subdomain. It inherits from `ElementSubdomainModifierBase`, which handles the corresponding

- Moving boundary/interface nodeset/sideset modification,
- Solution reinitialization, and
- Stateful material property reinitialization,

all of which are demonstrated below.

Consider a unit square domain decomposed by a vertical line $x=0.25$. The elements on the left side of the vertical line have subdomain ID of 1, and the elements on the right side have subdomain ID of 2. The `ElementSubdomainModifier` is used to change the subdomain ID from 2 to 1 for elements within a circle of radius 0.5 whose center moves along the bottom side towards the right.

## Moving boundary/interface nodeset/sideset modification

!alert note
If the moving boundary is defined completely around a subdomain, or between subdomains, then [SidesetAroundSubdomainUpdater.md] may be more useful to use in conjunction with the `ElementSubdomainModifier`, rather than using the `moving_boundaries` and `moving_boundary_subdomain_pairs` parameters in `ElementSubdomainModifier`.

The change of element subdomains will alter the definitions of certain sidesets and nodesets. The parameters `moving_boundaries` and `moving_boundary_subdomain_pairs` can optionally be used to modify the corresponding sidesets/nodesets over the elements that change subdomain. The names of the boundaries are specified in `moving_boundaries`, and the pair of subdomains that each boundary in `moving_boundaries` lies between must be specified in the corresponding `moving_boundary_subdomain_pairs`. The element side from the first subdomain of the pair is added to the boundary.

If the boundaries provided through `moving_boundaries` already exist, the modifier will attempt to modify the provided sidesets/nodesets whenever an element changes subdomain ID. If the boundaries do not exist, the modifier will create sidesets and nodesets with the provided names.

!listing test/tests/meshmodifiers/element_subdomain_modifier/moving_boundary.i start=[moving_circle] end=[] include-end=true

!media large_media/mesh_modifiers/element_subdomain_modifier/nodeset.png style=float:center;width:100%; caption=The evolving nodeset (green) between subdomains 1 and 2, as created by the modifier without an existing boundary. 

The modifier only creates and modifies boundaries over elements that change subdomain, so the vertical boundary between subdomains 1 and 2 at $x=0.25$ is not added to the created boundary. 

If only one boundary is provided but multiple pairs of subdomains are specified, then all the pairs are applied to the one boundary. Element sides on a subdomain's external boundary can also be added by specifying only one subdomain.

!listing test/tests/meshmodifiers/element_subdomain_modifier/external_moving_boundary.i start=[ext] end=[AuxVariables] include-end=false

!media large_media/mesh_modifiers/element_subdomain_modifier/ext.png style=float:center;width:100%; caption=The evolving sideset (green) around subdomain 1, including the external element sides, from an existing boundary.

Since the update of the moving boundary only occurs over elements that change subdomain, this can be used to update boundaries which do not cover the entirety of a subdomain:

!listing test/tests/meshmodifiers/element_subdomain_modifier/partial_moving_boundary.i start=[moving_circle] end=[] include-end=true

!media large_media/mesh_modifiers/element_subdomain_modifier/partial.png style=float:center;width:100%; caption=The evolving sideset (green) around subdomain 1, including the external element sides, from an existing boundary.
 

Even though the `moving_boundary_subdomain_pairs` defines the moving boundary to be between subdomains 1 and 2 only, the right side of subdomain 2 remains throughout, as no element sides belong to elements that change subdomain.

Nodal and integrated BCs can be applied on the moving boundary.

## Solution reinitialization

By default, all elements that change subdomain ID are reinitialized to the new subdomain's initial condition. Suppose the auxiliary variable $u$ has an initial variable value of $1$ in subdomain 1 and $-0.5$ in subdomain 2, and the variable value doubles at each timestep in subdomain 1:

!listing test/tests/meshmodifiers/element_subdomain_modifier/initial_condition.i start=[ICs] end=[Postprocessors]

!media large_media/mesh_modifiers/element_subdomain_modifier/init_cond.png style=float:center;width:100%; caption=The auxiliary variable $u$ is reinitialized to $1$, which doubles over the timestep to $2$, for all the elements that change subdomain ID to 1

## Stateful material property reinitialization

Similarly, all stateful material properties will be re-initialized when an element changes subdomain ID. Suppose initially the diffusivity is $0.5$ in subdomain 1 and $-1$ in subdomain 2, and the diffusivity doubles at each time step in subdomain 1:

!listing test/tests/meshmodifiers/element_subdomain_modifier/stateful_property.i start=[Materials] end=[Executioner]

!media large_media/mesh_modifiers/element_subdomain_modifier/stateful_prop.png style=float:center;width:100%; caption=The diffusivity is reinitialized to $0.5$, which doubles over the timestep to $1$, for all the elements that change subdomain ID to 1.

## Reinitialization restrictions

Depending on the physics, one may or may not want to reinitialize the solution when an element and its related nodes change subdomain ID. For the below examples, consider a unit square domain decomposed by vertical lines $x=0.3$ and $x=0.6$. The elements on the left have subdomain ID of 1, the elements in the middle have subdomain ID of 2, and the elements on the right have subdomain ID of 3.

The `ElementSubdomainModifier` is used to change the subdomain ID to 1 for elements within a circle of radius 0.3 whose center moves along the bottom side towards the right, and to subdomain ID 2 for elements within a circle of radius 0.3 whose center moves along the top side towards the right.

An auxiliary variable $u$ is defined over the domain, with initial values of 1, 2, and 3 in subdomains 1, 2, and 3 respectively:

!listing test/tests/meshmodifiers/element_subdomain_modifier/reinitialization.i start=[MeshModifiers] end=[AuxVariables]

!media large_media/mesh_modifiers/element_subdomain_modifier/orig.png style=float:center;width:100%; caption=The default behaviour of the modifier is to reinitializate all elements that change subdomain ID.

However, if a list of subdomains (IDs or names) is provided through the parameter `reinitialize_subdomains`, the reinitialization only occurs if the element's new subdomain ID is in the provided list:

!listing test/tests/meshmodifiers/element_subdomain_modifier/reinitialization_into.i start=[MeshModifiers] end=[AuxVariables]

!media large_media/mesh_modifiers/element_subdomain_modifier/into.png style=float:center;width:100%; caption=Reinitialization of only the elements that change subdomain ID to 1.

If an empty list is given in `reinitialize_subdomains`, then there is no reinitialization of any elements that change subdomain ID.

!listing test/tests/meshmodifiers/element_subdomain_modifier/no_reinitialization.i start=[MeshModifiers] end=[AuxVariables]

!media large_media/mesh_modifiers/element_subdomain_modifier/none.png style=float:center;width:100%; caption=No reinitialization of any elements that change subdomain ID.

Reinitialization can be further restricted by setting the parameter `old_subdomain_reinitialized` to `false`. The modifier will then additionally check the element's old subdomain ID. Reinitialization then only occurs if the old subdomain ID was not in the list provided in the parameter `reinitialize_subdomains`.

!listing test/tests/meshmodifiers/element_subdomain_modifier/reinitialization_from_into.i start=[MeshModifiers] end=[AuxVariables]

!media large_media/mesh_modifiers/element_subdomain_modifier/from_into.png style=float:center;width:100%; caption=Reinitialization of only the elements which change subdomain ID from 3, to subdomain IDs 1 or 2