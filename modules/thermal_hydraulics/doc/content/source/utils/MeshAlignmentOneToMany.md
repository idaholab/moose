# MeshAlignmentOneToMany

This is a base class for [MeshAlignment1D3D.md] and [MeshAlignment2D3D.md].
It builds a mapping between elements/faces for one of the following cases:

- 1D-to-3D: A 1D subdomain and a 3D boundary
- 2D-to-3D: A 2D boundary and a 3D boundary

Each element on the primary side maps to multiple elements on the secondary side.
