# NestedDivision

!syntax description /MeshDivisions/NestedDivision

The divisions are numbered from 0 to the product of the number of divisions/regions in
each of the divisions passed in the [!param](/MeshDivisions/NestedDivision/divisions) parameter.

The first division specified always corresponds to the outermost indexing. The last division specified rules
the innermost indexing. Each division object specified may have its own nesting of indexing. For example,
[CartesianGridDivision.md] nests indexing into the X, Y and Z bins.

!syntax parameters /MeshDivisions/NestedDivision

!syntax inputs /MeshDivisions/NestedDivision

!syntax children /MeshDivisions/NestedDivision
