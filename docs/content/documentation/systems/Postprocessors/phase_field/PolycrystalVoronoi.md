# PolycrystalVoronoi

The PolycrystalVoronoi UserObject generates a set of random points and performs a Voronoi tesslation to produce a grain structure. The number of grains may be set by the user but a suitable number must be chosen that is supported by the mesh resolution and grain boundary width. Once the grain structure has been generated, a coloring algorithm is used to assign order parameters to grains so that a reduced number of order parameters may be used. Typical numbers are given here:

!table style=border:4px solid black;width:350px;
| Dimension | Recommended OPs |
|-----------|-----------------|
| 2D        | 8               |
| 3D        | 25              |

See [Polycrystal Initial Conditions](ICs/PolycrystalICs.md) for more information.

## Typical usage in an input file:
!listing modules/phase_field/examples/grain_growth/grain_growth_2D_graintracker.i block=UserObjects link=false pre-style=max-height:450px

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_graintracker.i block=ICs


## Description and Syntax

!syntax description /Postprocessors/PolycrystalVoronoi

!syntax parameters /Postprocessors/PolycrystalVoronoi

!syntax inputs /Postprocessors/PolycrystalVoronoi

!syntax children /Postprocessors/PolycrystalVoronoi
