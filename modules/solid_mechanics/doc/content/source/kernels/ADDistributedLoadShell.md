# ADDistributedLoadShell

!syntax description /Kernels/ADDistributedLoadShell

## Description

The `ADDistributedLoadShell` kernel calculates the contribution to the residual from the distributed loads (in units of N/m²) on the shell elements. The load can be applied either in a specified direction within the global x-y-z coordinate system (e.g., self-weight, wind load) or normal to the shell plane (e.g., pressure loads). To determine if the distributed load should be treated as a pressure load (applied normal to the shell), the parameter 'project_load_to_normal=true' must be set. When this parameter is enabled, the load is applied perpendicularly to the shell surface. By default, project_load_to_normal=false, meaning the load is applied in the global coordinate directions.

The magnitude of the distributed load can be specified as either a scalar (use the input parameter factor, which defaults to 1.0), a function parameter, or a Postprocessor name. If more than one of these are given, they are multiplied by one another.

## Example Input File syntax

!syntax parameters /Kernels/ADDistributedLoadShell

!syntax inputs /Kernels/ADDistributedLoadShell

!syntax children /Kernels/ADDistributedLoadShell
