<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# MeshModifiers System

## Overview

- `MeshModifiers` further modify the mesh after it has been created.
- Possible modifications include : adding node sets, translating, rotating and scaling the mesh points.
- Users can create their own MeshModifiers by inheriting from `MeshModifier` and defining a custom `modify()` method.
- A few built-ins:

  - `AddExtraNodeSet`
  - `SideSetFromNormals` and `SideSetFromPoints`
  - `Transform` (Scale, Rotate, Translate)
  - `MeshExtruder`

## Extrusion

- `type = MeshExtruder`
- Takes a 1D or 2D mesh an extrudes it to 2D or 3D respectively,
- Triangles are extruded to prisms (wedges).
- Quadrilaterals are extruded to hexahedra.
- Sidesets are extruded and preserved.
- Newly-created top and bottom sidesets can be named by the user.
- The extrusion vector's direction and length must be specified.

!media large_media/mesh_modifiers/extrude.png
       caption=Extruded Mesh result from MAMBA courtesy of Michael Short
       style=width:50%;

## Further MeshModifiers Documentation

!syntax list /MeshModifiers objects=True actions=False subsystems=False

!syntax list /MeshModifiers objects=False actions=False subsystems=True

!syntax list /MeshModifiers objects=False actions=True subsystems=False

