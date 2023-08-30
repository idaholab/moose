# PolycrystalHex

The PolycrystalHex object generates a set of points in 2D space such that hexagonal patterns are generated from a resulting Voronoi Tesselation. The number of grains must be chosen such that the $\sqrt[dim=2]{n}$ root is an even integer. Furthermore, a suitable number must be chosen that is supported by the mesh resolution and grain boundary width. At present, the object is supported for generating hexagons in 2D and hexagonal prisms in 3D with the `columnar_3D` option. It works with periodic boundary conditions. Once the grain structure has been generated, a coloring algorithm is used to assign order parameters to grains so that a reduced number of order parameters may be used. Typical numbers are given here:

!table style=border:4px solid black;width:350px;
| Dimension | Recommended OPs |
|-----------|-----------------|
| 2D        | 8               |

See [Polycrystal Initial Conditions](ICs/PolycrystalICs.md) for more information.

## Typical usage in an input file:

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_graintracker.i block=UserObjects link=false

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_graintracker.i block=ICs


## Description and Syntax

# PolycrystalHex

!syntax description /Postprocessors/PolycrystalHex

!syntax parameters /Postprocessors/PolycrystalHex

!syntax inputs /Postprocessors/PolycrystalHex

!syntax children /Postprocessors/PolycrystalHex
