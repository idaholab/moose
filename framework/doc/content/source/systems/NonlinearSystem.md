# NonlinearSystem

The NonlinearSystem object holds the equation system created by the normal FEM process
(e.g. the Matrix and RHS vector) to be solved. Normally MOOSE uses PETSc to store and
solve this system. This object is where you will find the callback routines used
by the PETSc solvers.

One such routine is `NonlinearSystemBase::augmentSparsity`, which as its name
suggests augments the sparsity pattern of the matrix. Currently this method adds
sparsity coming from MOOSE `Constraint` objects. It does this by querying
geometric connectivity information between slave and master boundary pairs, and
then querying the `DofMap` attached to the `NonlinearSystemBase` (through the
libMesh `NonlinearImplicitSystem`) for the dof indices that exist on the
elements attached to the slave/master nodes. The geometric connectivity
information comes from [`NearestNodeLocators`](/NearestNodeLocator.md) held by
[`GeometricSearchData`](/GeometricSearchData.md) objects in the
[`FEProblemBase`](/FEProblemBase.md) and
[`DisplacedProblem`](/DisplacedProblem.md) (the latter only if there are mesh
displacements). In the future sparsity augmentation from constraints will occur
through [`RelationshipManagers`](/RelationshipManager.md) instead of through the
`augmentSparsity` method.
