# PolycrystalHex

The PolycrystalHex object generates a set of points in 2D or 3D space such that hexagonal patterns are generated from a resulting Voronoi Tesselation. The number of grains must be choosen such that the $\sqrt[dim]{n}$ root is an integer. Furthermore, a suitable number must be chosen that is supported by the mesh resolution and grain boundary width. Once the grain structure has been generated, a coloring algorithm is used to assign order parameters to grains so that a reduced number of order parameters may be used. Typical numbers are given here:

!table style=border:4px solid black;width:350px;
| Dimension | Recommended OPs |
|-----------|-----------------|
| 2D        | 8               |
| 3D        | 25              |

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
