# Engineering Units

Custom domain-specific engineering units can be defined such that they can be used in place of basic [!ac](CSG) components (surfaces, cells, and universes) in a `CSGBase` instance.
To define a custom engineering unit, create a class that derives from one of the engineering unit class types: `CSGSurfaceEngUnit` for surface-like units, `CSGCellEngUnit` for cell-like units, and `CSGUniverseEngUnit` for universe-like units.
Each of these classes derive from both the `CSGEngUnit` class and the corresponding base class (`CSGSurface`, `CSGCell`, and `CSGUniverse`, respectively).
Therefore, any defined engineering unit can behave as the base component from which it derives, while benefiting from the functionality and convenience of being an engineering unit.
A more detailed description of how to use engineering units in the `generateCSG` method can be found in [source/csg/CSGBase.md].

## Common Methods

All custom engineering units must implement the following two methods (first two methods are described in more detail below):

- `expandUnit`: creates the equivalent object using basic [!ac](CSG) components and adds them to the provided `CSGBase` instance
- `getAttributes`: returns a map of necessary unit attributes to their values
- `clone`: clones the instance as a unique pointer

All engineering units have additional information retrievable with the following methods:

- `getName`: returns the name of the unit
- `getBehavior`: returns what type of object this unit behaves like (`"SURFACE"`, `"CELL"`, or `"UNIVERSE"`)
- `getUnitType`: returns the class name as a string for the specific engineering unit

### Unit Expansion

All engineering units, regardless of type, must implement the `expandUnit` method for the engineering unit.
This method recreates the engineering unit as the corresponding rudimentary components (`CSGSurface`, `CSGCell`, `CSGUniverse`, and/or `CSGLattice`) and creates an object of the type of base component that it is being used as.
The implementation of this method follows the same basic guidelines for implementing the `generateCSG` method described in [source/csg/CSGBase.md].
More details on implementing this method for each of the three types of units is below in the respective sections.

!alert! note title=Naming Conventions and Restrictions

To avoid naming conflicts between units and other `CSGBase` components during the expansion, a good practice is to include the original unit's name (retrievable with `getName()`) as a part of the name for any generated component. Additionally, because the original unit is still present in the `CSGBase` instance until the full expansion process is complete, the final generated component cannot have the same name as the original unit. I.e., if a `CSGCellEngUnit` is named `my_cell`, the resulting `_expanded_cell` cannot also be named simply `my_cell`, but `my_cell_expanded` would be allowable.

!alert-end!

### Unit Attributes

The engineering unit attributes at a minimum are any pieces of data that are needed for a complete definition of the unit for any downstream connected codes or defining the expansion method.
For example, the minimum required information to include in the attributes for a regular N-sided polygon is the number of sides `N` and the `apothem` (center-to-flat distance).
These attributes are what get returned in a map using the `getAttributes()` method, which gets called when producing the [!ac](CSG) [!ac](JSON) object in [source/csg/CSGBase.md].

!alert! note title=Attributes Data Type

The `getAttributes()` method returns a map of `AttributeVariant` data types (`std::unordered_map<std::string, AttributeVariant>`) to support flexibility in the type of data that might need to be defined. The `AttributeVariant` is a `std::variant` that can hold any of the following types: `int`, `unsigned int`, `std::string`, `Real`, `bool`, and vectors of these types (`std::vector<int>`, `std::vector<unsigned int>`, `std::vector<std::string>`, `std::vector<Real>`, `std::vector<bool>`).

!alert-end!

### Setting the Unit Type

The type of unit must be set for `_unit_type` in the unit constructor.
It is recommended that this be done based on the class name using `MooseUtils::prettyCppType<EngUnitClassName>()` so that the unit type automatically matches the class that created it.

## `CSGSurfaceEngUnit`

To define a surface-like engineering unit, define a class the derives from `CSGSurfaceEngUnit`.
Functionally, these behave like a `CSGSurface` in that you can use them to define half-spaces when defining `CSGRegions` for a `CSGCell`.
All methods that are available on `CSGSurface` objects are also available on `CSGSurfaceEngUnit` objects except `getCoeffs` (not applicable to engineering units).

### Expansion Implementation

The implementation of the `expandUnit` method must set the value of `_expanded_region` such that it defines the "negative" half-space `CSGRegion` of the original unit.
The `_expanded_region` is returned by `getExpandedRegion` which is called during the expansion process via `CSGBase`.
If `expandUnit` has not been called or `_expanded_region` was not set properly in the implementation, the `getExpandedRegion` query will return an error.
To implement this, it is expected that additional `CSGSurfaces` are generated and added to the `CSGBase` object provided.

### Half-space Determination Implementation

All `CSGSurfaceEngUnits` are also derived from the `CSGSurface` and therefore must also implement the `evaluateSurfaceEquationAtPoint` method as described in [CSGSurface](source/csg/CSGSurface.md#half-space-determination).
At a minimum, this should return a negative real number if a point is in the negative half-space of the unit and a positive value for the positive half-space.
This value is used in `getHalfspaceFromPoint` in the same regard as standard `CSGSurface` objects.

## CSGCellEngUnit

To define a cell-like engineering unit, define a class the derives from `CSGCellEngUnit`.
Functionally, these behave like `CSGCell` objects in that you can add them to a `CSGUniverse`.
Because `CSGCellEngUnits` are defined by custom human-readable attributes, the standard `CSGCell` attributes such as a cell fill or region are undefined and unable to be updated via the standard methods available to `CSGCell`.

### Expansion Implementation

The implementation of the `expandUnit` method must set `_expanded_cell` to a pointer to a `CSGCell` object that defines the equivalent geometry.
The `_expanded_cell` is returned by `getExpandedCell` which is called during the expansion process via `CSGBase`.
If `expandUnit` has not been called or `_expanded_cell` was not set properly in the implementation, the `getExpandedCell` query will return an error.
To implement this, it is expected that `CSGSurfaces`, and possibly `CSGUniverses` or other `CSGCells`, are generated and added to the `CSGBase` object provided.

## CSGUniverseEngUnit

To define a universe-like engineering unit, define a class the derives from `CSGUniverseEngUnit`.
Functionally, these behave like a `CSGUniverse` object in that you can use them as fills for `CSGCells` or elements or outer for a `CSGLattice`.
The expectation is that a `CSGUniverseEngUnit` replaces a collection of `CSGCell` objects using human-readable information, and therefore, the method `addCell` are not applicable to this unit type like it is a `CSGUniverse`.

### Expansion Implementation

The implementation of the `expandUnit` method must set `_expanded_universe` to a pointer to a `CSGUniverse` object that defines the equivalent geometry.
The `_expanded_universe` is returned by `getExpandedUniverse` which is called during the expansion process via `CSGBase`.
If `expandUnit` has not been called or `_expanded_universe` was not set properly in the implementation, the `getExpandedCell` query will return an error.
To implement this, it is expected that `CSGCell` objects will be generated and added to the `CSGBase` object provided.

!alert! note title=Cell and Universe Relationships

It is important to note that when calling methods like `createCell` on the `CSGBase` object, by default the cell is added to the root universe of that `CSGBase` instance unless a `CSGUniverse` object is provided during the creation method. Therefore, when implementing the `expandUnit` method for a `CSGUniverse` where the generated `CSGUniverse` is expected to own the collection of generated cells (rather than the root universe), it is highly recommended to provide `_expanded_universe` as the "add-to" universe during the `createCell` call.

!alert-end!

## Example Implementation

Below shows how an engineering unit to define an N-Sided Regular Polygon (`CSGNPolygonUnit`) as a `CSGSurfaceEngUnit` is implemented as an example.

!listing /framework/include/csg/CSGNPolygonUnit.h

!listing /framework/src/csg/CSGNPolygonUnit.C
