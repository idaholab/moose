# Fluid-structure interface

!syntax description /InterfaceKernels/FluidStructureInterface

## Description

This interface kernel enforces displacement and pressure/stress continuity across
the fluid and structural domains. This interface kernel is part of the fluid-structure interaction codes. Please refer to [fluid-structure interaction using acoustics](/fsi_acoustics.md) for the theoretical details.

!alert note title=Element and neighbor definitions and normal vector
In this interface kernel, element is always the structure and neighbor is always
 the fluid. Furthermore, the normal vector always points from the structure to
 the fluid. This means, when defining the sideset using the `SideSetsBetweenSubdomainsGenerator` option in MOOSE meshing, `primary_block` should
  be the structure and `paired_block` should be the fluid. If sideset is defined
  using Paraview, use the `with respect to` option.

!syntax parameters /InterfaceKernels/FluidStructureInterface

!syntax inputs /InterfaceKernels/FluidStructureInterface

!syntax children /InterfaceKernels/FluidStructureInterface
