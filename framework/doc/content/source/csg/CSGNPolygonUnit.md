# N-Sided Regular Polygon Unit

The `CSGNPolygonUnit` is a built-in [engineering unit](source/csg/CSGEngUnit.md) that represents a regular N-sided polygon as a `CSGSurfaceEngUnit`.
It provides a convenient way to define a regular polygonal prism from two parameters, the number of sides and the apothem, instead of manually constructing and combining the individual planes that form each face.
General information on how engineering units are created, used, and expanded within a `CSGBase` instance can be found in [source/csg/CSGBase.md#engineering-units].

## Geometry and Orientation

A `CSGNPolygonUnit` represents a regular polygon that is infinite along the z-axis (i.e., a prismatic region).
By default, the polygon is centered at the origin with the right-most edge parallel to the y-axis, as shown in [!ref](fig:npoly-orientation).
This default orientation can be changed by applying [transformations](source/csg/CSGBase.md#transformations) to the unit.

!media large_media/csg/n-sided-poly-orientation.png
       id=fig:npoly-orientation
       caption=Depiction of the assumed default orientation of an N-sided polygon engineering unit.

The polygon is defined by $N$ infinite planes, one per side.
The $k$-th plane (where $k = 0, 1, \ldots, N-1$ and the 0th face is the right-most face) is described by the equation

!equation
x \cos\left(\frac{2 \pi k}{N}\right) + y \sin\left(\frac{2 \pi k}{N}\right) = A

where $A$ is the apothem (the center-to-flat distance).
In the general plane form $ax + by + cz = d$, this corresponds to coefficients $a = \cos(2 \pi k / N)$, $b = \sin(2 \pi k / N)$, $c = 0$, and $d = A$.
The interior of the polygon is the intersection of the $N$ negative half-spaces of these planes.

## Construction

A `CSGNPolygonUnit` is created like any other engineering unit by constructing a unique pointer and adding it to the `CSGBase` instance with `addEngUnit()` (see [source/csg/CSGBase.md#engineering-units]).
The constructor requires a unique name, the number of sides ($\ge 3$), and the apothem ($\gt 0$):

!listing CSGBaseTest.C start=define a 4-sided polygon end=addEngUnit include-end=true

!listing CSGBaseTest.C start=make a 4-sided polygon with apothem length 2.0 end=addEngUnit include-end=true

## Attributes

The `getAttributes()` method returns a map containing the two defining parameters of the polygon:

| Attribute   | Type   | Description                               |
|-------------|--------|-------------------------------------------|
| `num_sides` | `int`  | number of sides of the regular polygon    |
| `apothem`   | `Real` | distance from the center to a side (flat) |

These attributes are the minimum information needed to fully define the polygon for downstream connected codes and are what get written to the [!ac](CSG) [!ac](JSON) output when the unit is not expanded.

In addition to the attributes above, several convenience getter methods are provided to retrieve other geometric quantities derived from the number of sides and apothem:

- `getNumSides()`: returns the number of sides
- `getApothem()`: returns the apothem (center-to-flat distance)
- `getSideLength()`: returns the edge length, computed as $2 A \tan(\pi / N)$
- `getRadius()`: returns the circumradius (center-to-vertex distance), computed as $A / \cos(\pi / N)$

## Use as a Surface

Because `CSGNPolygonUnit` is a `CSGSurfaceEngUnit`, it can be used in place of a `CSGSurface` when defining the region of a `CSGCell`.
The "negative" half-space of the unit corresponds to the interior of the polygon.
The example below creates a square (a 4-sided polygon with apothem 2.0, i.e., a side length of 4.0) and uses its interior as the region of a material-filled cell:

!listing CSGBaseTest.C start=make a cell that uses the polygon unit end=createCell include-end=true

### Half-space Determination

As a `CSGSurfaceEngUnit`, the polygon implements `evaluateSurfaceEquationAtPoint`, which is used by `getHalfspaceFromPoint` to determine whether a point lies inside or outside the polygon.
For a point $(x, y)$, the method evaluates

!equation
x \cos\left(\frac{2 \pi k}{N}\right) + y \sin\left(\frac{2 \pi k}{N}\right) - A

for each side $k$ and returns the maximum value over all $N$ sides.
A point is interior to the polygon only if this value is negative for every side, so returning the maximum yields a negative value when the point is inside, a positive value when it is outside, and zero when it lies exactly on a side.
This evaluation uses the stored geometric parameters directly and therefore can be performed before the unit is expanded.

## Expansion

When a `CSGNPolygonUnit` is expanded (see [source/csg/CSGBase.md#expansion]), its `expandUnit()` implementation creates $N$ `CSGPlane` surfaces, one for each side of the polygon.
Each generated plane is named using the scheme `[UnitName]_expanded_surf_[k]`, where `k` is the side index.
For each plane, the half-space containing the origin is determined and intersected with the accumulating region so that final `CSGRegion` defines the interior of the polygon.
When the unit is expanded within a `CSGBase` instance, the unit is replaced by this new `CSGRegion` and the generated `CSGPlane` surfaces are added to the base.

## Example Use Case

The following is an end-to-end example of a mesh generator that produces an N-sided polygon unit, with the option to expand it into its rudimentary components.
Within the `generateCSG` method, the polygon unit is created and added to the `CSGBase` instance, used as the region of a material cell, and optionally expanded based on the `expand_unit` input parameter.

!listing TestPolygonUnitMeshGenerator.C start=// name of the current mesh generator end=return csg_obj include-end=true

When run without expansion, the engineering unit itself is the final output.
For example, the following input creates an infinite triangular prism (a 3-sided polygon with an apothem of 4):

!listing csg_only_poly_unit.i

This produces the [!ac](CSG) [!ac](JSON) output below, where the polygon unit is reported directly using its attributes (`num_sides` and `apothem`):

!listing csg_only_poly_unit_out_csg.json

When `expand_unit = true`, the polygon unit is removed from the output and replaced with the corresponding plane surfaces and the interior region:

!listing csg_only_poly_unit_expand.i

The resulting output shows the three generated `CSGPlane` surfaces (named following the `[UnitName]_expanded_surf_[k]` scheme) and the cell region defined by their intersection:

!listing csg_only_poly_unit_expand_out_csg.json
