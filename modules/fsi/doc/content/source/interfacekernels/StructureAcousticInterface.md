# Structure-acoustic interface

!syntax description /InterfaceKernels/StructureAcousticInterface

## Description

This interface kernel enforces displacement and pressure/stress continuity across
the fluid and structural domains. This interface kernel is part of the fluid-structure interaction codes. Please refer to [fluid-structure interaction using acoustics](/fsi_acoustics.md) for the theoretical details.

!alert note title=Element and neighbor definitions and normal vector
In this interface kernel, element is always the fluid and neighbor is always
 the structure. Furthermore, the normal vector always points from the fluid to
 the structure. This means, when defining the sideset using the `SideSetsBetweenSubdomainsGenerator` option in MOOSE meshing, `primary_block` should
  be the fluid and `paired_block` should be the structure. If sideset is defined
  using Paraview, use the `with respect to` option.

!syntax parameters /InterfaceKernels/StructureAcousticInterface

!syntax inputs /InterfaceKernels/StructureAcousticInterface

!syntax children /InterfaceKernels/StructureAcousticInterface
