# StatefulMaterialPropertyExporter

!syntax description /UserObjects/StatefulMaterialPropertyExporter

## Description

`StatefulMaterialPropertyExporter` serializes all volumetric and boundary stateful material
property data — together with the physical coordinates and grouping information of every
quadrature point — to a set of binary `.smatprop` files at the end of a simulation.  The exported data can later be loaded
by [StatefulMaterialPropertyImporter](StatefulMaterialPropertyImporter.md) to initialize the
old/older material states of a new simulation on a *different* mesh, using closest-point
mapping.

### File Naming and Parallel I/O

Each MPI rank writes its own file independently:

```
{file_base}.0.smatprop   (rank 0)
{file_base}.1.smatprop   (rank 1)
...
{file_base}.N-1.smatprop (rank N-1)
```

Every file carries the total rank count in its header so that the importer can discover the
complete set of files by reading only `{file_base}.0.smatprop`, and so that stale files from
a previous run with a different rank count are automatically excluded.

### File Format

Each `.smatprop` file contains:

1. **Header** — magic number, format version, and total rank count.
2. **Volumetric section** and **boundary section**, in that order.  Each section holds a
   property registry (property name, internal C++ type identifier, maximum state index for
   each stateful property of that storage) followed by the data, organized in groups.  For
   every quadrature point within a group: the physical coordinates followed by the serialized
   value for every stateful property at every state (current, old, older).

A group is identified by the subdomain name of the element, the sorted names of the boundaries
that contain the element side, and the subdomain name of the neighbor element across the side.
Volumetric data is grouped by subdomain name alone.  Boundary data covers face material
properties on exterior boundaries, internal sidesets, and interfaces, and is grouped by all
three names so that the importer never matches quadrature points across different boundaries
or across the two sides of an interface.

Serialization uses MOOSE's `dataStore`/`dataLoad` framework, which correctly handles
heap-allocated property types such as `std::vector<Real>`.

!alert note title=Stateful properties only
Only properties for which an "old" or "older" state has been requested somewhere in the
simulation are exported.  Non-stateful (current-only) properties are not written.

!alert note title=Neighbor properties are not exported
Stateful properties stored for the neighbor side of internal faces and interfaces are not
exported.

!alert note title=Current-state data is also written
The exporter writes state `0` (current), `1` (old), and `2` (older, if present)
for every exported property. The importer restores state `0` as well so that the first
transient shift preserves the imported history.

## Example Input Syntax

!listing test/tests/userobjects/stateful_material_remap/export.i block=UserObjects

Boundary stateful data is exported by the same object whenever face material properties are
stateful, for example on the exterior boundary `right` and the internal sideset `interface`
here:

!listing test/tests/userobjects/stateful_material_remap/export_boundary.i block=Postprocessors

The [!param](/UserObjects/StatefulMaterialPropertyExporter/file_base) parameter sets the base
name used for all output files.  The default
[!param](/UserObjects/StatefulMaterialPropertyExporter/execute_on) is `FINAL`, which writes
the data once after the last timestep has converged — the typical use case when handing off
state to a subsequent simulation.

## Workflow

The intended workflow is:

1. Run simulation A to convergence.  `StatefulMaterialPropertyExporter` writes
   `{file_base}.{rank}.smatprop` files at `EXEC_FINAL`.
2. Run simulation B (potentially on a different mesh and with a different number of MPI
   ranks).  [StatefulMaterialPropertyImporter](StatefulMaterialPropertyImporter.md) reads
   all rank files and remaps old/older states onto the new mesh via closest-point matching.

## Limitations

- Stateful neighbor material properties are not exported.

!syntax parameters /UserObjects/StatefulMaterialPropertyExporter

!syntax inputs /UserObjects/StatefulMaterialPropertyExporter

!syntax children /UserObjects/StatefulMaterialPropertyExporter
