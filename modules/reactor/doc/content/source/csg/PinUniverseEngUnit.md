# Pin Cell Universe Engineering Unit

`PinUniverseEngUnit` is a built-in [engineering unit](source/csg/CSGEngUnit.md) that represents a 2-D/infinite cylindrical pin structure.
It directly defines this structure by specifying the ring radii and radial region names of each incremental region.  When expanded, the [CSG](syntax/CSG/index.md)-based representation of the unit can be obtained that comprises the surfaces, cells, and universe needed to represent the underlying pin universe.
General information on how engineering units are created, used, and expanded within a `CSGBase` instance can be found in [source/csg/CSGBase.md#engineering-units].

## Construction

A `PinUniverseEngUnit` is created like any other engineering unit by constructing a unique pointer and adding it to the `CSGBase` instance with `addEngUnit()` (see [source/csg/CSGBase.md#engineering-units]).
The constructor requires a unique name, and the region names of each radial region. The region names are provided as a list, starting from the name of innermost ring region and expanding radially outwards. The number of region names should be one more than the number of ring radii, where the final region name is associated with the region outside of the outermost ring. The ring radii should be provided in ascending order.

!listing DuctedPinEngUnit.C start=Create a PinUniverseEngUnit engineering unit and add it to CSGBase end=addEngUnit include-end=true

## Attributes

The `getAttributes()` method returns a map containing the parameters of the pin engineering unit:

| Attribute    | Type                       | Description                             |
|--------------|----------------------------|-----------------------------------------|
| `ring_radii` | `std::vector<Real>`        | List of ring radii of infinite pin cell |
| `fill_mats`  | `std::vector<std::string>` | List of fill material names of pin cell |

## Expansion

When a `PinUniverseEngUnit` is expanded (see [source/csg/CSGBase.md#expansion]), its `expandUnit()` implementation creates a single universe that represents the pin cell. The universe is defined by concentric CSG cylinder surfaces aligned with the z-axis for each ring radius, and a cell that is defined for each incremental radial region. Each of these cells has a material fill with the name corresponding to that region.
