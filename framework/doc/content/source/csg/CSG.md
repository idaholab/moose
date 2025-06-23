# Constructive Solid Geometry

[!ac](CSG) is a geometry representation in which complex models are created through boolean combinations of surfaces, cells, and universes.
[!ac](CSG) models are most commonly used for [!ac](MC) neutronics simulations.
While each [!ac](MC) code has their own syntax for defining the [!ac](CSG) model, the underlying theory for creating [!ac](CSG) representations is the same throughout.
The `CSGBase` class provides the framework in MOOSE for creating these generic [!ac](CSG) representations of mesh generators that can then be used by [!ac](MC) codes.

## Theory

As stated, a [!ac](CSG) representation is defined minimally as a series of surfaces, cells, and universes.
This section describes in theory what these components are.

[Surfaces](source/csg/CSGBase.md#surfaces) are defined explicitly through surface equations (such as equations of a plane, sphere, etc.).
Each surface inherently separates two halfspace [regions](source/csg/CSGBase.md#regions): positive and negative halfspaces.
For example, for a plane with the equation `Ax + By + Cz = D` the positive halfspace represents the region `Ax + By + Cz > D`, while the negative halfspace represents the region `Ax + By + Cz < D`. Similarly, for a spherical surface defined by the equation `x^2 + y^2 + z^2 = r^2`, the negative halfspace represents the region `x^2 + y^2 + z^2 < r^2` within the sphere while the positive halfspace represents the region `x^2 + y^2 + z^2 > r^2` outside the sphere.
Example halfspaces are shown in [!ref](fig:halfspaces).

!media large_media/csg/halfspaces.png
       id=fig:halfspaces
       caption=Example depiction of the positive and negative halfspaces defined by a plane (left) and sphere (right).

These halfspace regions defined by the surfaces can be combined further series of boolean operators for unions, intersections, and complements to further define more complex regions.
For example, if we wanted to use the surfaces from [!ref](fig:halfspaces) to define just the left hemisphere, we would define the cell region as the intersection of the negative halfspace of the plane and the negative halfspace of the sphere ([!ref](fig:intersection)).

!media large_media/csg/region_intersection.png
       id=fig:intersection
       caption=Example depiction of a closed region defined by an intersection of two halfspaces.

[Cells](source/csg/CSGBase.md#cells) are defined by two main characteristics: a region and a fill.
The region is defined as described above and defines the domain of the cell.
The fill can typically be set as void (i.e., nothing), a material (typically specified by provided the name of the material), a [universe](source/csg/CSGBase.md#universes), or a lattice (note, lattices are not yet supported for MOOSE implementation).

[Universes](source/csg/CSGBase.md#universes) can then be optionally defined as a collection of cells, which can then be used to either fill other cells, or used repeatedly throughout a geometry (such as in a repeated lattice).

## How to Invoke

The [!ac](CSG) model generation can be invoked at the command line using the `--csg-only` option with any MOOSE mesh input file.
The [!ac](JSON) file that is generated will be called the name of the input file with `_csg` appended by default.
An optional output file name can be provided at the command line (`--csg-only <output_file_name.json>`).

If all mesh generator blocks in the input file have the `generateCSG` method implemented, a [!ac](CSG)-equivalent [!ac](JSON) file will be produced.
If any mesh generators do not have the `generateCSG` method implemented, an error will be returned explaining as such.
This process is run as a data-only mode so no finite element mesh is produced.

## Output

Calling `--csg-only` will produce a [!ac](JSON) file.

