# SidesetAroundSubdomainUpdater

## Description

The `SidesetAroundSubdomainUpdater` mesh modifier updates a sideset during a simulation by reassigning it based on a pair of subdomain sets, the [!param](/MeshModifiers/SidesetAroundSubdomainUpdater/inner_subdomains) and [!param](/MeshModifiers/SidesetAroundSubdomainUpdater/outer_subdomains). The sideset will be updated to comprise all sides along `inner_subdomains` that are neighboring `outer_subdomains` or that have no neighbor. This dynamic update can be useful in conjunction with subdomain update user objects such as [CoupledVarThresholdElementSubdomainModifier](CoupledVarThresholdElementSubdomainModifier.md).  

 An optional 'mask' can be used to restrict the sides which can be added to the moving side as the boundary moves, [!param](/MeshModifiers/SidesetAroundSubdomainUpdater/mask_side), as shown in:

!listing test/tests/meshmodifiers/sideset_around_subdomain_updater/simple.i block=MeshModifiers

!alert note
Use the [!param](/MeshModifiers/SidesetAroundSubdomainUpdater/execution_order_group) parameter to ensure that this user object is run after any subdomain changing user object has fully traversed the mesh.

!alert warning
Nodesets are also updated as well as sidesets, so the [!param](/Mesh/FileMesh/construct_node_list_from_side_list) `Mesh` parameter will not be respected.


## Example Input Syntax

!listing test/tests/meshmodifiers/sideset_around_subdomain_updater/test.i block=MeshModifiers

!syntax parameters /MeshModifiers/SidesetAroundSubdomainUpdater

!syntax inputs /MeshModifiers/SidesetAroundSubdomainUpdater

!syntax children /MeshModifiers/SidesetAroundSubdomainUpdater
