# PETScDMDAMesh

Generate a parallel (distributed) mesh from PETSc DMDA.  DMDA could be passed in from an application such as ExternalPetscSolverApp or created on the fly. Note that this mesh object does not have one layer of ghost elements. It is designed for holding the solution from an external PETSc application. And then the solution can be coupled to other MOOSE-based applications using the existing MultiApp transfers.
