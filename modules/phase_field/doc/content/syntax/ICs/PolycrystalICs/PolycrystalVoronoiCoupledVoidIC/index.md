# PolycrystalVoronoiCoupledVoidIC System

## Overview

The PolycrystalVoronoiCoupledVoidIC generates a Voronoi tesslation to produce a grain structure around voids. The void distribution is obtained via a coupled variable, which must be initialized using a separate initial condition class like [MultiSmoothCircleIC](/MultiSmoothCircleIC) or [SpecifiedSmoothCircleIC](/SpecifiedSmoothCircleIC). The voids are not restricted along the grain boundaries alone as in [PolycrystalVoronoiVoidIC](/PolycrystalVoronoiVoidIC) The centroids of grains can be either generated from a set of random points or assigned from a file. It requires the number of voids to be greater than zero. In general, you should use [PolycrystalVoronoi](/PolycrystalVoronoi.md) to represent Voronoi grain structures without voids.

## Example Input File Syntax

!listing modules/phase_field/test/tests/initial_conditions/PolycrystalVoronoiVoidIC_periodic.i

!syntax list /ICs/PolycrystalICs/PolycrystalVoronoiCoupledVoidIC objects=True actions=False subsystems=False

!syntax list /ICs/PolycrystalICs/PolycrystalVoronoiCoupledVoidIC objects=False actions=False subsystems=True

!syntax list /ICs/PolycrystalICs/PolycrystalVoronoiCoupledVoidIC objects=False actions=True subsystems=False

!template load file=stubs/moose_system.md.template name=PolycrystalColoringIC syntax=/ICs/PolycrystalICs/PolycrystalColoringIC
