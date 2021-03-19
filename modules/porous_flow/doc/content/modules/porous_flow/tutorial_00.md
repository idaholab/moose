# Porous Flow Tutorial Page 00

# Contents

- [Page 01](tutorial_01.md) A single fluid
- [Page 02](tutorial_02.md) Numerical issues
- [Page 03](tutorial_03.md) Adding heat advection and conduction
- [Page 04](tutorial_04.md) Adding solid mechanics
- [Page 05](tutorial_05.md) Using a realistic equation of state for
   the fluid
- [Page 06](tutorial_06.md) Adding a tracer
- [Page 07](tutorial_07.md)  A chemically reactive tracer with porosity and permeability changes
- [Page 08](tutorial_08.md)  The `PorousFlowSink` and unsaturated flow
- [Page 09](tutorial_09.md)  An overview of the PorousFlow architecture
- [Page 10](tutorial_10.md)  Unleashing the full power of PorousFlow: using Kernels and Materials
- [Page 11](tutorial_11.md)  Two-phase THM borehole injection
- [Page 12](tutorial_12.md) Boundary sinks and sources, and polyline
  sinks and sources (boreholes and rivers)
- [Page 13](tutorial_13.md) More elaborate chemistry


# Introduction

The PorousFlow module is a library of physics for fluid and heat flow
in porous media. It is formulated in a general manner, so is capable
of solving problems with an arbitrary number of phases, fluid
components and chemical reactants.  This tutorial guides the user
through some commonly-used aspects of PorousFlow.  Lots of core
documentation may be found in [Porous Flow](porous_flow/index.md).



This tutorial concerns fluid injection through a borehole into a large
fluid-filled reservoir.  The borehole is vertical, and cylindrical
symmetry around the borehole axis is assumed.  The tutorial begins
with simple Darcy flow, and gradually adds more complex phenomena such
as coupling with heat and solid mechanics, multi-phase flows and
chemical reactions.

After conceptual modelling, the first step in any finite-element
simulation is to create the mesh.  The 3D mesh used for this tutorial is
deliberately coarse (so that the tutorial input files may be easily
run on the smallest computers) and is generated using MOOSE's inbuilt
meshing capabilities.  Firstly, a 2D annular mesh is created:

!listing modules/porous_flow/examples/tutorial/00.i start=[Mesh] end=[make3D]

The radius of the borehole is 1$\,$m, the radius of the model is
10$\,$m, and only $1/4$ of the annulus is considered.

Now a sequence of `MeshModifiers` are applied to this 2D mesh.  First,
it is extruded to be 12$\,$m high using:

!listing modules/porous_flow/examples/tutorial/00.i start=[make3D] end=[shift_down]

Then it is shifted downwards (ie, in the negative $z$ direction) by
6$\,$m to place the injection region around the origin:

!listing modules/porous_flow/examples/tutorial/00.i start=[shift_down] end=[aquifer]

An aquifer region is created in the central 6$\,$m:

!listing modules/porous_flow/examples/tutorial/00.i start=[aquifer] end=[injection_area]

and an injection area is created on the borehole's wall in the aquifer region:

!listing modules/porous_flow/examples/tutorial/00.i start=[injection_area] end=[rename]

Finally, the subdomains are named "caps" (for the upper and lower
caprock) and "aquifer":

!listing modules/porous_flow/examples/tutorial/00.i start=[rename] end=[]

This process has created the 3D mesh.  Each of the input files in the
tutorial follow this process.  The result is shown in [tut00.fig].

!media tut00.png style=width:50%;margin-left:10px caption=The 3D mesh.  The aquifer is shown in red and the caprocks in blue.  The green surface is the injection area.  id=tut00.fig

If this were a real simulation rather than just a tutorial, it would
be much more efficient to use cylindrical coordinates, which are
called "RZ" coordinates in MOOSE.  The mesh-generation process is

!listing modules/porous_flow/examples/tutorial/00_2D.i start=[Mesh] end=[Variables]
