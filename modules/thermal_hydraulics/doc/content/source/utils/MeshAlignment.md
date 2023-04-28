# MeshAlignment

This class builds a mapping between one of the following:

- 1D-to-2D: A 1D subdomain and a 2D boundary
- 2D-to-2D: A 2D boundary and other 2D boundary

These "meshes" must be *aligned*, i.e., each element/side on the primary mesh
must correspond to a unique element/side on the secondary mesh. The pairing is
determined by checking the centroids of the elements/sides and finding the
nearest match on the other mesh.
