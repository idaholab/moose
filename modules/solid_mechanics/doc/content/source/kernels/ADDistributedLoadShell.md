# ADDistributedLoadShell

!syntax description /Kernels/ADDistributedLoadShell

## Description

The `ADDistributedLoadShell` kernel calculates the contribution to the residual from a distributed load (in units of force per area) on the shell elements. The load can be applied either in a specified direction within the global x-y-z coordinate system (e.g., self-weight, wind load) or normal to the shell plane (e.g., pressure loads). The `project_load_to_normal` parameter defines how the direction of the the load is defined. This parameter defaults to `false`, which indicates that the load is applied in the direction of the variable this is applied to (defined by the `variable` parameter). If `project_load_to_normal=true`, the distributed load is treated as a pressure load applied in the shell normal direction, perpendicular to the shell surface. For this case, an instance of this kernel is required for each of the displacement variables.

The magnitude of the distributed load is specified as a function. A scalar value can be supplied to the function (i.e., `function=1.0`) for constant loading.

## Example Input File syntax

!syntax parameters /Kernels/ADDistributedLoadShell

!syntax inputs /Kernels/ADDistributedLoadShell

!syntax children /Kernels/ADDistributedLoadShell
