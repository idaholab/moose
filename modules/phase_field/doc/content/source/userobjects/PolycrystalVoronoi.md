# PolycrystalVoronoi

The PolycrystalVoronoi UserObject either generates a set of random points or
reads a set of grain centroids from a file and performs a Voronoi tesslation to
produce a grain structure. The number of grains may be set by the user but a
suitable number must be chosen that is supported by the mesh resolution and
grain boundary width. Once the grain structure has been generated, a coloring
algorithm is used to assign order parameters to grains so that a reduced number
of order parameters may be used. Typical numbers are given here:

| Dimension | Recommended OPs |
|-----------|-----------------|
| 2D        | 8               |
| 3D        | 25              |

See [Polycrystal Initial Conditions](ICs/PolycrystalICs.md) for more information.

!alert note title=Use KDTree to improve performance
To reduce the computational cost associated with setting up a polycrystal structure with a large number of grains, an alternative option is to use the `KDTree` approach. In this case, a search tree is built a-priori such that the time to search nearest points and neighboring grains is optimized. To leverage this approach for an initial condition with diffused interfaces, it is recommended that users determine what is the minimum `grain_patch_size` required for their problem (the default value is `grain_patch_size = 10`).

## Typical usage in an input file:

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_graintracker.i block=UserObjects link=false

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_graintracker.i block=ICs

An example of activating `KDTree` is provided below:

!listing modules/phase_field/test/tests/initial_conditions/PolycrystalVoronoiIC_periodic.i block=UserObjects


## Description and Syntax

!syntax description /Postprocessors/PolycrystalVoronoi

!syntax parameters /Postprocessors/PolycrystalVoronoi

!syntax inputs /Postprocessors/PolycrystalVoronoi

!syntax children /Postprocessors/PolycrystalVoronoi
