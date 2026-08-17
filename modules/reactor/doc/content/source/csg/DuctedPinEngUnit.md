# Ducted Pin Engineering Unit

`DuctedPinEngUnit` is a built-in [engineering unit](source/csg/CSGEngUnit.md) that represents a 2-D/infinite or 3-D cylindrical pin structure surrounded by one or more hexagonal or square ducts.
It directly defines this structure by specifying the apothems (center-to-flat distances) of the ducts, the ring radii, the radial and axial region names of each pin region, and the geometry type ("Hex" or "Square") of the pin cell. When expanded, the [CSG](syntax/CSG/index.md)-based representation of the unit can be obtained that comprises the surfaces, cells, and universes needed to represent the underlying ducted pin cell structure.
General information on how engineering units are created, used, and expanded within a `CSGBase` instance can be found in [source/csg/CSGBase.md#engineering-units].

## Construction

A `DuctedPinEngUnit` is created like any other engineering unit by constructing a unique pointer and adding it to the `CSGBase` instance with `addEngUnit()` (see [source/csg/CSGBase.md#engineering-units]).
The constructor requires a unique name, the geometry type ("Hex" or "Square"), the ring radii, the duct apothems, the region names, and the size of each axial layer, from bottom to top. For 2-D pin cell structures, an empty list should be passed into the final parameter. The region names are provided as a 2-D list, where the inner index represents the radial region names (starting from the innermost radial ring and expanding radially outwards to the outermost ducted region), and the outer index represent the axial region names (starting from the bottom most axial layer and incrementing axially upward). The duct apothems and ring radii should be provided in ascending order, with the final duct apothem representing the half-pitch of the entire pin cell structure. The axial boundaries of the pin cell assume that the lowest axial plane starts at z = 0.

!listing PinMeshGenerator.C start=Define pin engineering unit and add it to CSGBase end=addEngUnit include-end=true

## Attributes

The `getAttributes()` method returns a map containing the parameters of the ducted pin cell:

| Attribute          | Type                                    | Description                                   |
|--------------------|-----------------------------------------|-----------------------------------------------|
| `duct_apothems`    | `std::vector<Real>`                     | List of duct apothems of ducted pin cell      |
| `ring_radii`       | `std::vector<Real>`                     | List of ring radii of ducted pin cell         |
| `region_names`     | `std::vector<std::vector<std::string>>` | Radial and axial region names                 |
| `geometry_type`    | `std::string`                           | Geometry type ("Hex" or "Square") of pin cell |
| `axial_boundaries` | `std::vector<Real>`                     | Size of axial layers for 3-D pin cell         |

Note: `axial_boundaries` will exist in the map only if axial geometry information was provided to the `DuctedPinEngUnit` constructor.

## Expansion

When a `DuctedPinEngUnit` is expanded (see [source/csg/CSGBase.md#expansion]), its `expandUnit()` implementation creates a single cell that constrains the pin domain defined by the outermost duct boundary and the top and bottom axial plans. This cell has a universe fill that contains the concentric hexagonal cells that represent the hexagonal/square ducts for each axial region. The inner-most duct cell is filled by a [PinUniverseEngUnit](PinUniverseEngUnit.md) to represent the concentric cylindrical rings of the pin, while the rest of the duct cells contains a material fill with a user-specific region name. Each of the concentric hexgonal/square ducts is defined using [CSGNPolygonUnit](CSGNPolygonUnit.md).
