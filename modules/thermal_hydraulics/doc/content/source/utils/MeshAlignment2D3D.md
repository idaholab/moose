# MeshAlignment2D3D

This class inherits from [MeshAlignmentOneToMany.md] and builds a mapping between
elements/faces between a 2D boundary and a 3D boundary.

Each element on the primary side maps to multiple elements on the secondary side.
These "meshes" must be *aligned*; the 2D boundary is associated
with an arbitrary axis, and the axial coordinates of the paired elements must
match.
