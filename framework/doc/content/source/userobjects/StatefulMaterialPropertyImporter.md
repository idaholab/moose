# StatefulMaterialPropertyImporter

!syntax description /UserObjects/StatefulMaterialPropertyImporter

## Description

`StatefulMaterialPropertyImporter` reads a set of `.smatprop` binary files written by
[StatefulMaterialPropertyExporter](StatefulMaterialPropertyExporter.md) and remaps the
exported volumetric and boundary stateful material property data onto the current simulation
mesh using closest-point matching within matching subdomains and boundaries.  This allows the converged material history of one simulation to be
used as the initial history for a subsequent simulation on a *different* mesh — for example
after remeshing, mesh refinement, or mesh transfer between application codes.

### Closest-Point Mapping

For each quadrature point in the current mesh the importer finds the nearest exported
quadrature point *within the same subdomain* using a [KDTree](https://en.wikipedia.org/wiki/K-d_tree)
(one tree per subdomain, built from the physical coordinates stored in the export files).
The state values of that nearest point are then staged for the current element and loaded
through MOOSE's stateful material restart path.

Subdomains are matched by name.  If the current mesh contains a subdomain whose name does
not appear in any of the export files, the elements in that subdomain are silently skipped
and their stateful properties are initialized by the material's `initStatefulProperties()`
as normal.

### Boundary Data

Boundary (face) stateful data is matched the same way, but within groups of element sides that
share the subdomain name of the element, the sorted names of the boundaries that contain the
side, and the subdomain name of the neighbor element across the side.  Including the boundary
names keeps quadrature points near a corner from matching a different boundary of the same
subdomain, and including both subdomain names keeps the two sides of an interface apart.
Element sides whose group does not appear in the export files are initialized as normal.

Data is staged for every element side whose group appears in the export files.  Sides on which
the current simulation does not initialize stateful boundary data leave the staged data unused.

!alert note title=Imported properties may be skipped
Properties present in the export files but not declared as stateful properties in the
current simulation are skipped. The reverse — having additional stateful
properties in the current simulation that are absent from the files — is also allowed;
those properties receive their normal custom initialization from
`initStatefulProperties()`.

### Type and State Safety

The property type identifiers stored in the file headers are compared against those registered
in the current simulation.  A `mooseError` is raised if the types do not match, preventing
silent data corruption. Diagnostics print demangled type names for readability.

The exported and current simulations must also request the same maximum state index for each
imported property. For example, a target simulation that requests `older` data cannot import a
file that only contains `old` data for that property.

### Timing and Initialization Order

The importer stages its remapped data during `EXEC_INITIAL`, after the first
`initElementStatefulProps()` pass in `FEProblemBase::initialSetup()` but before the second pass
that reinitializes stateful properties. The staged values are then loaded through
`MaterialPropertyStorage`'s normal restart path while the second `initStatefulProperties()`
pass runs on each element. This preserves the first-pass initialization for non-imported
properties while restoring imported state in place.

This ordering ensures that materials with a mix of imported and non-imported stateful
properties are handled correctly: the first initialization pass establishes normal values for
all properties, and the second pass reloads only the imported ones through the restart path.

### Parallel I/O

The importer reads all rank files written by the exporter
(`{file_base}.0.smatprop`, `{file_base}.1.smatprop`, …) and merges their quadrature point
data into the per-group KDTrees before performing any search.  The number of files to
read is determined from the rank count stored in `{file_base}.0.smatprop`, so the import
works correctly when the current simulation uses a different number of MPI ranks than the
export simulation.

## Example Input Syntax

A basic import onto a different mesh:

!listing test/tests/userobjects/stateful_material_remap/import.i block=UserObjects

The [!param](/UserObjects/StatefulMaterialPropertyImporter/file_base) parameter must match
the [!param](/UserObjects/StatefulMaterialPropertyExporter/file_base) used by the exporter.
The [!param](/UserObjects/StatefulMaterialPropertyImporter/execute_on) parameter is fixed to
`EXEC_INITIAL` and cannot be changed.

A partial import — where the material declares additional stateful properties that are not in
the export file — is also supported:

!listing test/tests/userobjects/stateful_material_remap/import_partial.i block=UserObjects

Boundary data needs no additional input; it is imported whenever the export files contain it
and the current simulation declares the same stateful face properties:

!listing test/tests/userobjects/stateful_material_remap/import_boundary.i block=UserObjects

## Limitations

- Stateful neighbor material properties are not remapped.
- Nearest-point copy only; there is no interpolation or distance-based acceptance criterion.
- Every rank reads and stores the full exported point cloud before remapping, so import
  memory use and file I/O scale with the global export size on every rank.

## Related Objects

- [StatefulMaterialPropertyExporter](StatefulMaterialPropertyExporter.md) — writes the
  `.smatprop` files consumed by this object.

!syntax parameters /UserObjects/StatefulMaterialPropertyImporter

!syntax inputs /UserObjects/StatefulMaterialPropertyImporter

!syntax children /UserObjects/StatefulMaterialPropertyImporter
