# SCMTriCoreMeshGenerator

!syntax description /Mesh/SCMTriCoreMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator places triangular-lattice subchannel assemblies on a hexagonal core lattice and
merges them into one mesh. The assemblies are not connected: there are no inter-assembly gaps and no
crossflow between assemblies.

Each entry of [!param](/Mesh/SCMTriCoreMeshGenerator/inputs) is an
[SCMTriAssemblyMeshGenerator.md] or an [SCMTriDuctMeshGenerator.md] for a subchannel core mesh, or
an [SCMDetailedTriAssemblyMeshGenerator.md] for a detailed 3D core mesh used to visualize core
solutions. A detailed core built with the same pattern and pitch as a subchannel core places each
detailed assembly over the subchannel assembly with the same index. Each assembly is generated
centered at the origin and is translated so that its center lies on its lattice position. The
inputs may differ in size and number of pins, as long as no two placed assemblies overlap. The
overlap check is exact for the hexagonal ducts, and ducts that exactly touch are allowed.

[!param](/Mesh/SCMTriCoreMeshGenerator/pattern) uses the row syntax of `PatternedHexMeshGenerator`:
each entry is an index into [!param](/Mesh/SCMTriCoreMeshGenerator/inputs), and `-1` marks an empty
position. Subchannel assemblies have a duct corner on the $+x$ axis, so each pattern row is placed as
a column of assemblies. Row $i$ is the $i$-th column from $-x$, and the entries of a row are placed
from $-y$ to $+y$. Neighboring positions are
[!param](/Mesh/SCMTriCoreMeshGenerator/assembly_pitch) apart.

Placed assemblies are numbered $k = 0, 1, \ldots$ in pattern order, skipping empty positions. Every
subdomain and boundary of assembly $k$ receives a new id and has `_k` appended to its name, for
example `subchannel_0`, `fuel_pins_0`, `inlet_0`, and `outlet_0`. Every element of assembly $k$ also
stores $k$ in the `assembly_id` extra element integer.

!alert warning
Each subchannel assembly and duct mesh generator stores the subchannel connectivity in a single mesh
object, so the subchannel solvers cannot use the core mesh yet. A core of those inputs can only be
generated with `--mesh-only`. A detailed core has no such restriction.

## Example Input File Syntax

!listing /test/tests/mesh/tri_core/core.i block=TriSubChannelMesh language=moose

The detailed counterpart of this core is:

!listing /test/tests/mesh/tri_core/detailed_core.i block=Mesh language=moose

!syntax parameters /Mesh/SCMTriCoreMeshGenerator

!syntax inputs /Mesh/SCMTriCoreMeshGenerator

!syntax children /Mesh/SCMTriCoreMeshGenerator
