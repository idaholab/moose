# Polycrystal Initial Conditions

Polycrystal Initial Conditions can be created in a few ways. They can be read from a file, generated
with Voronoi Tesselation of a set of points, or placed in some regular crystal pattern such as hex or
circles. In any case, it is advantageous to simulate such a system with a reduced number of order
parameters for efficiency, which requires the use of the [Grain Tracker](/GrainTracker.md). The
trickiest part of running a reduced order parameter model however, is the initial assignment of
order parameters to grains. This process is akin to solving a
[graph coloring](https://en.wikipedia.org/wiki/Graph_coloring). Fortunately, the order parameter
assignment process has been taken care of for you. Each polycrystal initial condition should simply
extend the PolycrystalUserObjectBase class.

!media media/phase_field/voronoi_flood.png
       caption=Grain structure with associated neighbor graph overlaid.

Extentions of this class must begin by providing reporting the number of grains in the initial
condition.

!listing modules/phase_field/include/userobjects/PolycrystalUserObjectBase.h line=getNumGrains()

Additionally, the developer must provide an implementation for reporting the grain(s) at every point
in the domain:

!listing modules/phase_field/include/userobjects/PolycrystalUserObjectBase.h start=getGrainsBasedOnPoint end=/*

Finally, a method to report the variable value of the current order parameter at a point must be
provided. This method is called after order parameters have been assigned to all grains.

!listing modules/phase_field/include/userobjects/PolycrystalUserObjectBase.h line=getVariableValue(unsigned

The object uses these implementations to build a grain adjacency graph that can be feed to a
stochastic or deterministic graph coloring algorithm. MOOSE defaults to using one of the built-in
high performance coloring algorithms from the PETSc package. However, a simple backtracking algorithm
is also included which works reasonably well on smaller to mid-sized problems.

The centroids of grains can be randomly generated or read from a file. The use of Maximal Poisson-Disk Sampling (MPS) to generate grain centroids is described in [MPS](/MPS.md).

See:

[PolycrystalVoronoi](/PolycrystalVoronoi.md)

[PolycrystalEBSD](/PolycrystalEBSD.md)

[PolycrystalCircles](/PolycrystalCircles.md)

[PolycrystalHex](/PolycrystalHex.md)
