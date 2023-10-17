# ModularGapConductanceConstraint

!syntax description /Constraints/ModularGapConductanceConstraint

The `ModularGapConductanceConstraint` class is used specify a heat flux across a
gap.  The flux is computed by user objects derived from `GapFluxModelBase`. Such as

- [GapFluxModelSimple](GapFluxModelSimple.md)
- [GapFluxModelConduction](GapFluxModelConduction.md)
- [GapFluxModelRadiation](GapFluxModelRadiation.md)
- [GapFluxModelPressureDependentConduction](GapFluxModelPressureDependentConduction.md)

Multiple models can be specified with their contributions getting summed up.

## Gap geometry types

Plate, cylindrical, and spherical gap geometries can be specified using the [!param](/Constraints/ModularGapConductanceConstraint/gap_geometry_type) parameter.

For cylindrical geometries, e.g. `gap_geometry_type = cylinder`, the axis of the cylinder can be specified using the [!param](/Constraints/ModularGapConductanceConstraint/cylinder_axis_point_1) and [!param](/Constraints/ModularGapConductanceConstraint/cylinder_axis_point_2) parameters. In 2D Cartesian coordinate systems (e.g. `Mesh/coord_type=XYZ`), these parameters can be omitted to automatically determine the cylinder axis. This is accomplished by averaging positions over the primary side set, which should yield the center of the cylinder, assuming the side set spans the entire 360 degree rotation around the theoretical axis. The axis is then extended in z-direction.

For spherical geometries the origin of the sphere can be specified using the [!param](/Constraints/ModularGapConductanceConstraint/sphere_origin) parameter. If the parameter is omitted, the sphere origin is deduced by averaging positions over the primary side set.

!alert note
If the gap is sliced by a symmetry plane (e.g. only a quarter of a cylinder or an eighth of a sphere are modeled), the positional average over the primary side set will not yield the correct origin or axis and they will need to be supplied manually.

!syntax parameters /Constraints/ModularGapConductanceConstraint

!syntax inputs /Constraints/ModularGapConductanceConstraint

!syntax children /Constraints/ModularGapConductanceConstraint

!bibtex bibliography
