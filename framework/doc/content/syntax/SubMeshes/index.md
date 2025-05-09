# SubMeshes

The `SubMeshes` syntax is used to create MFEM [ParSubMesh](https://docs.mfem.org/html/classmfem_1_1ParSubMesh.html#details) objects, for the restriction of MFEM finite element spaces and outputs to subspaces of the parent mesh. Such subspaces can be constructed from either a set of mesh volumes or surfaces, and may be used for the definition of variables that are not defined over all elements in the parent mesh.
