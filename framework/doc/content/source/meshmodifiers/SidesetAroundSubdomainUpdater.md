# SidesetAroundSubdomainUpdater

## Description

The `SidesetAroundSubdomainUpdater` mesh modifier updates a sideset during a simulation by reassigning it based on a pair of subdomain sets, the [!param](/MeshModifiers/SidesetAroundSubdomainUpdater/inner_subdomains) and [!param](/MeshModifiers/SidesetAroundSubdomainUpdater/outer_subdomains). The sideset will be updated to comprise all sides along `inner_subdomains` that are neighboring `outer_subdomains` or that have no neighbor. This dynamic update can be useful in conjunction with subdomain update user objects such as [CoupledVarThresholdElementSubdomainModifier](CoupledVarThresholdElementSubdomainModifier.md).

!alert note
Use the [!param](/MeshModifiers/SidesetAroundSubdomainUpdater/execution_order_group) parameter to ensure that this user object is run after any subdomain changing user object has fully traversed the mesh.

## Example Input Syntax

!listing test/tests/meshmodifiers/sideset_around_subdomain_updater/test.i block=MeshModifiers

!syntax parameters /MeshModifiers/SidesetAroundSubdomainUpdater

!syntax inputs /MeshModifiers/SidesetAroundSubdomainUpdater

!syntax children /MeshModifiers/SidesetAroundSubdomainUpdater
