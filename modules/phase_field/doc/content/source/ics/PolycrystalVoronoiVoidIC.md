# PolycrystalVoronoiVoidIC

The PolycrystalVoronoiVoidIC generates a Voronoi tesslation to produce a grain structure with voids randomly distributed along the grain boundaries. The centroids of grains can be either generated from a set of random points or assigned from a file. It requires the number of voids to be greater than zero. In general, you should use [PolycrystalVoronoi](/PolycrystalVoronoi.md) to represent Voronoi grain structures without voids.

## Typical usage in an input file:

!listing modules/phase_field/test/tests/initial_conditions/PolycrystalVoronoiVoidIC_periodic.i block=ICs/c_IC

## Description and Syntax

!syntax description /ICs/PolycrystalVoronoiVoidIC

!syntax parameters /ICs/PolycrystalVoronoiVoidIC

!syntax inputs /ICs/PolycrystalVoronoiVoidIC

!syntax children /ICs/PolycrystalVoronoiVoidIC
