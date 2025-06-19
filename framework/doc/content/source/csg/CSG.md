# Constructive Solid Geometry

Constructive solid geometry (CSG) is a geometry representation in which complex models are created through boolean combinations of surfaces, cells, and universes.
CSG models are most commonly used for Monte Carlo (MC) neutronics simulations.
While each MC code has their own syntax for defining the CSG model, the underlying theory is the same throughout.
The `CSGBase` class provides the framework in MOOSE for creating these generic CSG representations of mesh generators that can then be used by MC codes.

## Theory

As stated, a CSG representation is defined minimally as a series of surfaces, cells, and universes.
This section describes in theory what these components are.

Surfaces are defined explicitly through surface equations (such as equations of a plane, sphere, etc.).
Each surface inherently separates two halfspace regions: positive and negative halfspaces.
For example, for a plane with the equation `Ax + By + Cz = D` the positive halfspace represents the region `Ax + By + Cz > D`, while the negative halfspace represents the region `Ax + By + Cz < D`. Similarly, for a spherical surface defined by the equation `x^2 + y^2 + z^2 = r^2`, the negative halfspace represents the region `x^2 + y^2 + z^2 < r^2` within the sphere while the positive halfspace represents the region `x^2 + y^2 + z^2 > r^2` outside the sphere.
Example halfspaces are shown in [!ref](fig:halfspaces).

!media large_media/csg/halfspaces.png
       id=fig:halfspaces
       caption=Example depiction of the positive and negative halfspaces defined by a plane (left) and sphere (right).

These halfspace regions defined by the surfaces can be combined using series of boolean operators for unions, intersections, and complements to further define more complex regions.
For example, if we wanted to use the surfaces from [!ref](fig:halfspaces) to define just the left hemisphere, we would define the cell region as the intersection of the negative halfspace of the plane and the positive halfspace of the sphere ([!ref](fig:intersection)).

!media large_media/csg/region_intersection.png
       id=fig:intersection
       caption=Example depiction of a closed region defined by an intersection of two halfspaces.

These halfspace regions defined by the surfaces are combined using series of boolean operators for unions, intersections, and complements to define complete regions.
For example, if we wanted to use the surfaces from Figure 1 (FIGURE OUT HOW TO REF) to define just the left hemisphere of the cell, we would define the cell region as the intersection of the negative halfspace of the plane and the positive halfspace of the sphere.

!row!

!col! class=s12 m6 l6

!media large_media/csg/region_intersection.png
       id=intersection
       caption=Example depiction of a closed region defined by an intersection of two halfspaces.

!col-end!

!row-end!

Cells are defined by two main characteristics: a region and a fill.
The region is defined as described above and defines the boundary of the cell.
The fill can typically be set as void, a material, a universe, or a lattice.

Universes can then be optionally defined as a collection of cells, which can then be used to either fill other cells, or used repeatedly throughout a geometry (such as in a repeated lattice).

# For Developers

The `CSGBase` class contains the framework necessary for creating generic CSG definitions, but the methods for actually using it to generate the CSG definition have to be implemented for each mesh generator by overriding the `generateCSG()` class.


overview of what the CSGBase framework does and how developers should implement this (ie for each MeshGenerator)

## generateCSG

specifics about this method - how to invoke, what to create, etc

## Example Implementation

provide an example of building a basic geometry using the CSGBase

## Output

description of the JSON structure and what is provided

# How to Invoke

explanation of how to invoke csg-only from the command line