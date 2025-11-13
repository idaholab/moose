# Constructive Solid Geometry

[!ac](CSG) is a geometry representation in which models are created through boolean combinations of surfaces, cells, and universes.
[!ac](CSG) models are most commonly used for [!ac](MC) neutronics simulations.
While each [!ac](MC) code has their own syntax for defining the [!ac](CSG) model, the underlying theory for creating [!ac](CSG) representations is the same throughout.
The `CSGBase` class provides the framework in MOOSE for creating these generic [!ac](CSG) representations of geometries through MOOSE mesh generators. These can then be used by [!ac](MC) codes.

## Theory

As stated, a [!ac](CSG) representation is defined minimally as a series of surfaces, cells, and universes.

[Surfaces](source/csg/CSGBase.md#surfaces) are defined explicitly through surface equations (such as equations of a plane, sphere, etc.).
Each surface inherently separates two half-space [regions](source/csg/CSGBase.md#regions): positive and negative half-spaces.
For example, for a plane with the equation $ax + by + cz = d$ the positive half-space represents the region $ax + by + cz > d$, while the negative half-space represents the region $ax + by + cz < d$. Similarly, for a spherical surface defined by the equation $x^2 + y^2 + z^2 = r^2$, the negative half-space represents the region $x^2 + y^2 + z^2 < r^2$ within the sphere while the positive half-space represents the region $x^2 + y^2 + z^2 > r^2$ outside the sphere.
Example half-spaces are shown in [!ref](fig:halfspaces).

!media large_media/csg/halfspaces.png
       id=fig:halfspaces
       caption=Example depiction of the positive and negative half-spaces defined by a plane (left) and sphere (right).

These half-space regions defined by the surfaces can be combined further as series of boolean operators for unions, intersections, and complements to further define more complex regions.
For example, if we wanted to use the surfaces from [!ref](fig:halfspaces) to define just the left hemisphere, we would define the cell region as the intersection of the negative half-space of the plane and the negative half-space of the sphere ([!ref](fig:intersection)).

!media large_media/csg/region_intersection.png
       id=fig:intersection
       caption=Example depiction of a closed region defined by an intersection of two half-spaces.

[Cells](source/csg/CSGBase.md#cells) are defined by two main characteristics: a region and a fill.
The region is defined as described above and defines the domain of the cell.
The fill can typically be set as void (i.e., nothing), a material (placeholder type for now, this will be expanded in future works), a [universe](source/csg/CSGBase.md#universes), or a [lattice](source/csg/CSGBase.md#lattices).

[Universes](source/csg/CSGBase.md#universes) can then be optionally defined as a collection of cells, which can then be used to either fill other cells, or used repeatedly throughout a geometry (such as in a repeated lattice).
By default, every model will have a [root universe](source/csg/CSGBase.md#root-universe), which is the singular overarching universe that all other universes can be traced back to through the tree defined by universes containing cells and cells filled with universes.

[Lattices](source/csg/CSGBase.md#lattices) are another optional component.
A lattice is the arrangement of repeated universes in a defined structure.
Some common types are regular Cartesian or hexagonal lattices.

For a more detailed description of how surfaces, cells, universes, and lattices are represented within the MOOSE mesh generator system, please refer to [source/csg/CSGBase.md].

## How to Generate a CSG Model

The [!ac](CSG) model generation can be invoked on the command line using the `--csg-only` option with any MOOSE mesh input file.
The [!ac](JSON) file that is generated will be called the name of the input file with `_csg` appended by default.
An optional output file name can be provided on the command line (`--csg-only <output_file_name.json>`).

!alert note title=Code requirements
A [!ac](CSG) [!ac](JSON) file will be produced if and only if all mesh generators in the input file have the `generateCSG` method implemented.
If any mesh generators do not have the `generateCSG` method implemented, an error will be returned explaining as such.
This process is run as a [data-only mode](source/meshgenerators/MeshGenerator.md#using-data-driven-generation) so no finite element mesh is produced.

## Output

Calling `--csg-only` will produce a [!ac](JSON) file containing the complete geometric description of the mesh generator input.
There are four main sections in the file:

- [`surfaces`](#surfaces)
- [`cells`](#cells)
- [`universes`](#universes)
- [`lattices`](#lattices) (if any)

Each item within each section is keyed (a [!ac](JSON) is a dictionary) by its unique name identifier, and the value is the corresponding definition for that item.
Detailed description of each type of item in the section follows.

### Surfaces

Surfaces contain the following information:

- `type`: the type of surface as defined by the class name that was used to create the surface
- `coefficients`: the values for each coefficient in the equation defining the surface type (shown in the table below)

| `type`      | Equation                                        | `coefficients`          |
|-------------|-------------------------------------------------|-----------------------|
| `CSG::CSGPlane`     | $ax + by + cz = d$                              | `a`, `b`, `c`, `d`    |
| `CSG::CSGSphere`    | $(x - x_0)^2 + (y - y_0)^2 + (z - z_0)^2 = r^2$ | `x0`, `y0`, `z0`, `r` |
| `CSG::CSGXCylinder` | $(y - y_0)^2 + (z - z_0)^2 = r^2$               | `y0`, `z0`, `r`       |
| `CSG::CSGYCylinder` | $(x - x_0)^2 + (z - z_0)^2 = r^2$               | `x0`, `z0`, `r`       |
| `CSG::CSGZCylinder` | $(x - x_0)^2 + (y - y_0)^2 = r^2$               | `x0`, `y0`, `r`       |

Below is an example [!ac](JSON) surface output for a model with a `CSG::CSGPlane` at $x=-2.0$ and a `CSG::CSGPlane` at $y=-2.0$.

!listing csg_only_inf_planes_out_csg.json start="surfaces" end="inf_square_surf_plus_x"

### Cells

The cells output contains the following information:

- `region`: the string representation of the equation of boolean operators (listed below) and surface names that defines the cell region
- `filltype`: type of fill in the cell (`"VOID"`, `"CSG_MATERIAL"`, `"UNIVERSE"`, or `"LATTICE"`)
- `fill`: for `"CSG_MATERIAL"`, `"UNIVERSE"`, or `"LATTICE"` `filltype`, the name of the fill object (if `"VOID"` type, then name is an empty string `""`)

| Boolean Operator   | String Representation |
|--------------------|-----------------------|
| positive half-space | `+`                   |
| negative half-space | `-`                   |
| union              | `|`                   |
| intersection       | `&`                   |
| complement         | `~`                   |

An example of a cell defined as the space inside a box made of six planes and filled with a solid material is below:

!listing csg_only_chained_out_csg.json start="cells" end="surfaces"

### Universes

Universes are simply defined by the list of the names of the `cells` that are contained in that universe.
If the universe is also the root universe, it will have the designator `"root": true`.
An example of universe output for multiple universes containing various concentric cylinder cells is below.
In this example, there is one universe, the default `"ROOT_UNIVERSE"`, which contains one cell and is labeled as being the root universe.

!listing csg_only_chained_out_csg.json start="universes"

### Lattices

Lattice output contains the following information:

- `type`: the type of lattice as defined by the class name that was used to create the lattice
- `universes`: the array of universe names that define the lattice arrangement.
- any other attributes associated with the specific lattice type (e.g., pitch, number of rows, etc.)

Two types of lattices are directly supported in the [source/csg/CSGBase.md] class: `CSG::CSGCartesianLattice` and `CSG::CSGHexagonalLattice`.
The additional attributes that are provided for each of these types are:

- `CSGCartesianLattice`:

  - `nx0`: the number of rows
  - `nx1`: the number of columns
  - `pitch`: the flat-to-flat distance of a single lattice element

- `CSGHexagonalLattice`:

  - `nring`: number of rings (consistent with the number of rows)
  - `nrow`: the number of rows (consistent with the number of rings)
  - `pitch`: the flat-to-flat distance of a single lattice element

Below is example output for a $2 \times 2$ Cartesian lattice filled with two different universes called `sq1_univ` and `sq2_univ`.

!listing csg_lattice_cart_out_csg.json start="lattices" end="surfaces"
