# MeshDivisionFunctorReductionVectorPostprocessor

!syntax description /VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor

This object is designed to allow a lot of flexibility in its uses.
Because it leverages [Functors](syntax/Functors/index.md), it can perform reductions on many different quantities/fields.
Because it uses a [MeshDivision](syntax/MeshDivisions/index.md), it can split the mesh in many different ways.
Because it uses a [!param](/VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor/reduction) parameter
instead of hard-coding a reduction, it can perform several operations like element integrations, averages, or the search
for an extrema.

For example, this object is strictly equivalent to a [NearestPointIntegralVariablePostprocessor.md] with:
- a variable for its [!param](/VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor/functors) parameter
- a [NearestPositionsDivision.md] for its [!param](/VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor/mesh_division) parameter (and the Positions object should be selected to match the points wanted for the nearest-point regions)
- and 'integral' for the [!param](/VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor/reduction) parameter


## Out-of-bounds behavior

If there are no elements within the subdomains of the `MeshDivisionFunctorReductionVectorPostprocessor`
for some of the bins of the user-specified [!param](/VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor/mesh_division),
the returned value of the reduction will be:

- 0 for an average and an integral reduction
- the maximum floating point number (a very large positive number, depending on the `Real` floating point number type used) for a minimum
- the minimum floating point number (minus the number mentioned above) for a maximum

If there are elements within the subdomains of the `MeshDivisionFunctorReductionVectorPostprocessor` that
are outside the user-specified [!param](/VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor/mesh_division),
a warning will be output and the elements will be ignored.

!syntax parameters /VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor

!syntax inputs /VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor

!syntax children /VectorPostprocessors/MeshDivisionFunctorReductionVectorPostprocessor
