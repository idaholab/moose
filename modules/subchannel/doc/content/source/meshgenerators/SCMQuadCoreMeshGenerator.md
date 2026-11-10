# SCMQuadCoreMeshGenerator

!syntax description /Mesh/SCMQuadCoreMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator places square-lattice subchannel assemblies on a rectangular core lattice and
merges them into one mesh. The assemblies are not connected: there are no inter-assembly gaps and no
crossflow between assemblies. It is the square-lattice counterpart of [SCMTriCoreMeshGenerator.md].

Each entry of [!param](/Mesh/SCMQuadCoreMeshGenerator/inputs) is an
[SCMQuadAssemblyMeshGenerator.md] or an [SCMQuadDuctMeshGenerator.md] for a subchannel core mesh,
or an [SCMDetailedQuadAssemblyMeshGenerator.md] for a detailed 3D core mesh used to visualize core
solutions. A detailed core built with the same pattern and pitch as a subchannel core places each
detailed assembly over the subchannel assembly with the same index.

Each assembly is generated centered at the origin and is translated so that its center lies on its
lattice position. The inputs may differ in size, including `nx` and `ny`, as long as no two placed
assemblies overlap. The outer duct of an assembly is $(n_x - 1)\,p + 2 g$ wide and
$(n_y - 1)\,p + 2 g$ high, where $p$ is the subchannel `pitch` and $g$ the `side_gap`. Two ducts
overlap when their center offset is shorter than the sum of their half widths in $x$ and shorter
than the sum of their half heights in $y$; ducts that exactly touch are allowed. With all positions
filled, the ducts therefore fit if every duct fits inside an
[!param](/Mesh/SCMQuadCoreMeshGenerator/assembly_pitch) by
[!param](/Mesh/SCMQuadCoreMeshGenerator/assembly_pitch) square.

[!param](/Mesh/SCMQuadCoreMeshGenerator/pattern) uses the row syntax of
`PatternedCartesianMeshGenerator`: each entry is an index into
[!param](/Mesh/SCMQuadCoreMeshGenerator/inputs), and `-1` marks an empty position. The lattice may
have any number of rows and columns, but all rows must have the same length. The first row is placed
at $+y$, the entries of a row are placed from $-x$ to $+x$, and the lattice is centered on the
origin. Neighboring positions are [!param](/Mesh/SCMQuadCoreMeshGenerator/assembly_pitch) apart in
$x$ and in $y$.

Placed assemblies are numbered $k = 0, 1, \ldots$ in pattern order, skipping empty positions. Every
subdomain and boundary of assembly $k$ receives a new id and has `_k` appended to its name, for
example `subchannel_0`, `fuel_pins_0`, `inlet_0`, and `outlet_0`. Every element of assembly $k$ also
stores $k$ in the `assembly_id` extra element integer.

!alert warning
Each subchannel assembly and duct mesh generator stores the subchannel connectivity in a single mesh
object, so the subchannel solvers cannot use the core mesh yet. A core of those inputs can only be
generated with `--mesh-only`. A detailed core has no such restriction.

## Example Input File Syntax

!listing /test/tests/mesh/quad_core/core.i block=QuadSubChannelMesh language=moose

The detailed counterpart of this core is:

!listing /test/tests/mesh/quad_core/detailed_core.i block=Mesh language=moose

!syntax parameters /Mesh/SCMQuadCoreMeshGenerator

!syntax inputs /Mesh/SCMQuadCoreMeshGenerator

!syntax children /Mesh/SCMQuadCoreMeshGenerator
