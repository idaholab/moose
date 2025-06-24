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
For example, for a plane with the equation $ax + by + cz = d$ the positive halfspace represents the region $ax + by + cz > d$, while the negative halfspace represents the region $ax + by + cz < d$. Similarly, for a spherical surface defined by the equation $x^2 + y^2 + z^2 = r^2$, the negative halfspace represents the region $x^2 + y^2 + z^2 < r^2$ within the sphere while the positive halfspace represents the region $x^2 + y^2 + z^2 > r^2$ outside the sphere.
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
By default, every model will have a [root universe](source/csg/CSGBase.md#root-universe), which is the singular overarching universe that all other universes can be traced back to through the tree defined by universes containing cells and cells filled with universes.

## How to Invoke

The [!ac](CSG) model generation can be invoked at the command line using the `--csg-only` option with any MOOSE mesh input file.
The [!ac](JSON) file that is generated will be called the name of the input file with `_csg` appended by default.
An optional output file name can be provided at the command line (`--csg-only <output_file_name.json>`).

If all mesh generator blocks in the input file have the `generateCSG` method implemented, a [!ac](CSG)-equivalent [!ac](JSON) file will be produced.
If any mesh generators do not have the `generateCSG` method implemented, an error will be returned explaining as such.
This process is run as a data-only mode so no finite element mesh is produced.

## Output

Calling `--csg-only` will produce a [!ac](JSON) file containing the complete geometric description of the mesh generator input.
There are three main sections in the file:

- [`SURFACES`](#surfaces)
- [`CELLS`](#cells)
- [`UNIVERSES`](#universes)

Each item within the blocks is keyed by its unique name identifier, and the value is the corresponding definition for that item.
Detailed description of each type of item in the section follows.

### Surfaces

Surfaces contain the following information:

- `TYPE`: the type of surface
- `COEFFICIENTS`: the values for each coefficient in the equation defining the surface type (shown in the table below)
- `BOUNDARY`: type of boundary, either `transmission` or `vacuum`

| `TYPE`      | Equation                                        | COEFFICIENTS          |
|-------------|-------------------------------------------------|-----------------------|
| `plane`     | $ax + by + cz = d$                              | `a`, `b`, `c`, `d`    |
| `sphere`    | $(x - x_0)^2 + (y - y_0)^2 + (z - z_0)^2 = r^2$ | `x0`, `y0`, `z0`, `r` |
| `xcylinder` | $(y - y_0)^2 + (z - z_0)^2 = r^2$               | `y0`, `z0`, `r`       |
| `ycylinder` | $(x - x_0)^2 + (z - z_0)^2 = r^2$               | `x0`, `z0`, `r`       |
| `zcylinder` | $(x - x_0)^2 + (y - y_0)^2 = r^2$               | `x0`, `y0`, `r`       |

Below is example [!ac](JSON) surface output for a `plane` at x=5, `zcylinder` of radius 2 centered at $(1, 3)$, and a `sphere` at $(-2, -3, 7)$ with radius 5.

```json
{
  "SURFACES": {
    "my_plane": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "a": 1.0,
        "b": 0.0,
        "c": 0.0,
        "d": 5.0
      },
      "TYPE": "plane"
    },
    "my_z_cylinder": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "r": 2.0,
        "x0": 1.0,
        "y0": 3.0
      },
      "TYPE": "zcylinder"
    },
    "my_sphere": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "r": 5.0,
        "x0": -2.0,
        "y0": -3.0,
        "z0": 7.0
      },
      "TYPE": "sphere"
    }
  }
}
```

### Cells

The cells output contains the following information:

- `REGION`: the string representation of the equation of boolean operators (listed below) and surface names
- `FILLTYPE`: type of fill in the cell (`VOID`, `MATERIAL`, or `UNIVERSE`)
- `FILL`: for `MATERIAL` or `UNIVERSE` `FILLTYPE`, the name of the fill (if `VOID` type, then name is an empty string `""`)

| Boolean Operator   | String Representation |
|--------------------|-----------------------|
| positive halfspace | `+`                   |
| negative halfspace | `-`                   |
| union              | `|`                   |
| intersection       | `&`                   |
| complement         | `~`                   |

An example of a cell defined as the space inside a finite x-cylinder and filled with a solid material is below:

```json
{
  "CELLS": {
    "My_XCylinder_Cell": {
      "FILL": "my_material",
      "FILLTYPE": "MATERIAL",
      "REGION": "(-xcyl_surf_0 & -x_pos_plane & +x_neg_plane)"
    }
  }
}
```

### Universes

Universes are simply defined by the list of the names of the `CELLS` that are contained in that universe.
If the universe is also the root universe, it will have the designator `"ROOT": true`.
An example of universe output for multiple universes containing various concentric cylinder cells is below.
In this example, the cells named `AllCylsUniverse_box`, `XCyls_cell`, `YCyls_cell`, and `ZCyls_cell` are filled with other universes listed, forming a tree of connectedness tracing back to the `ROOT_UNIVERSE`.

```json
{
  "UNIVERSES": {
    "ROOT_UNIVERSE": {
      "CELLS": [
        "AllCylsUniverse_box"
      ],
      "ROOT": true
    },
    "AllCyls_univ": {
      "CELLS": [
        "XCyls_cell",
        "YCyls_cell",
        "ZCyls_cell"
      ]
    },
    "XCyls_univ": {
      "CELLS": [
        "XCyls_cell_cyl_x_0",
        "XCyls_cell_cyl_x_1",
        "XCyls_cell_cyl_x_2"
      ]
    },
    "YCyls_univ": {
      "CELLS": [
        "YCyls_cell_cyl_y_0",
        "YCyls_cell_cyl_y_1"
      ]
    },
    "ZCyls_univ": {
      "CELLS": [
        "ZCyls_cell_cyl_z_0",
        "ZCyls_cell_cyl_z_1"
      ]
    }
  }
}
```
