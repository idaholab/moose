# StatefulMaterialPropertyExporter

!syntax description /UserObjects/StatefulMaterialPropertyExporter

## Description

`StatefulMaterialPropertyExporter` serializes all stateful material property data — together
with the physical coordinates and subdomain names of every quadrature point — to a set of
binary `.smatprop` files at the end of a simulation.  The exported data can later be loaded
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

1. **Header** — magic number, format version, total rank count, and a property registry
   (property name, demangled C++ type string, maximum state index for each stateful property).
2. **Data** — organized by subdomain name.  For every quadrature point within a subdomain:
   the physical coordinates followed by the serialized value for every stateful property at
   every state (current, old, older).

Serialization uses MOOSE's `dataStore`/`dataLoad` framework, which correctly handles
heap-allocated property types such as `std::vector<Real>`.

!alert note title=Stateful properties only
Only properties for which an "old" or "older" state has been requested somewhere in the
simulation are exported.  Non-stateful (current-only) properties are not written.

!alert note title=Volumetric properties only
The current implementation exports volumetric (interior) properties only (`side = 0`).
Boundary/face material properties are not yet supported.

## Example Input Syntax

!listing test/tests/userobjects/stateful_material_remap/export.i block=UserObjects

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

!syntax parameters /UserObjects/StatefulMaterialPropertyExporter

!syntax inputs /UserObjects/StatefulMaterialPropertyExporter

!syntax children /UserObjects/StatefulMaterialPropertyExporter
