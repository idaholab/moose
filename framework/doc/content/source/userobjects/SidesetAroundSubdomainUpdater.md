# SidesetAroundSubdomainUpdater

!syntax description /UserObjects/SidesetAroundSubdomainUpdater

## Description

The `SidesetAroundSubdomainUpdater` user object updates a sideset during a simulation by reassigning it based on a pair of subsomain sets, the [!param](/UserObjects/SidesetAroundSubdomainUpdater/inner_subdomains) and [!param](/UserObjects/SidesetAroundSubdomainUpdater/outer_subdomains). The sideset will be updated to comprise all sides along `inner_subdomains` that are neighboring `outer_subdomains` or that have no neighbor. This dynamic update can be useful in conjunction with subdomain update user objects such as [CoupledVarThresholdElementSubdomainModifier](CoupledVarThresholdElementSubdomainModifier.md).

!alert note
Use the [!param](/UserObjects/SidesetAroundSubdomainUpdater/execution_order_group) parameter to ensure that this user object is run after any subdomain changing user object has fully traversed teh mesh.

## Example Input Syntax

!listing test/tests/auxkernels/solution_aux/solution_aux_exodus_interp.i block=UserObjects

!syntax parameters /UserObjects/SidesetAroundSubdomainUpdater

!syntax inputs /UserObjects/SidesetAroundSubdomainUpdater

!syntax children /UserObjects/SidesetAroundSubdomainUpdater
