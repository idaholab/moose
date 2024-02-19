# MeshDivisions

The `MeshDivisions` system is designed to be able to subdivide the mesh arbitrarily.
It associates a contiguously numbered single-indexing to regions of the mesh.
It can match many of the pre-existing ways of sub-dividing the mesh:

- using subdomains with [SubdomainsDivision.md]
- using extra element integers with [ExtraElementIntegerDivision.md]
- using a nearest-neighbor algorithm with [NearestPositionsDivision.md]

Some new simple ways to subdivide the mesh:

- using the values of a [Functor](Functors/index.md) with [FunctorBinnedValuesDivision.md]
- using a Cartesian grid with [CartesianGridDivision.md]
- using a cylindrical grid with [CylindricalGridDivision.md]
- using a spherical grid with [SphericalGridDivision.md]

Divisions can be combined through nesting, using a [NestedDivision.md]. The onus lies
on the user to have the nesting make sense.

!alert note
An alternative option to distribute a division object would be to use a [Positions](syntax/Positions/index.md)
object within the definition of the object, for example the center of a [CylindricalGridDivision.md].
This has not been implemented yet. Please reach out to a MOOSE developer if this is of interest.

## Indexing the entire mesh or not

Each `MeshDivision` object can keep track of whether the entire mesh is indexed by the `MeshDivision`.
This can be expensive to check at any point, because the mesh could deform or because the bins
of the `MeshDivision` for the divisions change. Each `MeshDivision` object should either perform
a rigorous check before considering that the entire mesh is indexed, or make a conservative assumption
that the entire mesh is not indexed in the division.

## Postprocessing with MeshDivisions

For now, the mesh divisions can only be output using a [MeshDivisionAux.md].
We are planning to be able to compute averages, integrals, and various reductions
on mesh divisions, please stay tuned.

## Transferring with MeshDivisions

This feature is a work in progress.

!alert note
Please let us know on [GitHub Discussions](https://github.com/idaholab/moose/discussions)
how you are using the `MeshDivisions` system so we can include your techniques on this page!

## Multi-dimensional indexing

This feature is not implemented. We always boil down nested binning (in X, Y and Z for example)
down to a single division index. If you cannot do so for your problem, we could consider introducing multi-dimensional
indexes. Please get in touch with a MOOSE developer to discuss this.
