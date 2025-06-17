# CSGBase

`CSGBase` is the main class with which developers should interact with when implementing the `generateCSG` method for any mesh generator.
This class acts as a container and driver for all methods necessary for creating a CSG representation such as generating surfaces, cells, and universes.

## Initialization

A new `CSGBase` object can be initialized with:

```cpp
auto csg_obj = std::make_unique<CSG::CSGBase>();
```

Once initialized, surfaces, cells, and universes can be created and manipulated.
For example:

```cpp
// create a universe to contain all the cells created
auto new_univ = csg_obj->createUniverse('my_new_universe');
// create a sphere surface with radius 5 at the origin
auto sphere_at_origin = csg_obj->createSphere('my_new_sphere', 5);
// create a void cell whose region is the inside of the sphere and add it to the new universe
auto sphere_cell = csg_obj->createCell('my_new_cell', -sphere_at_origin, new_univ);
```

## Surface Methods

Various methods exist to create `CSGSurface` objects (below).
All surface creation methods will return a shared pointer to that generated surface.

| Surface | Method | Description |
|---------|--------|------------|
| Plane | `createPlaneFromPoints` | create a plane defined by 3 points |
| Plane | `createPlaneFromCoefficients` | creates a plane based on the coefficients `a`, `b`, `c`, and `d` for the equation `ax + by + cz = d` |
| Sphere | `createSphere` | creates a sphere of radius `r` at an optionally specified center point (default is `(0, 0, 0)`) |
| Axis-Aligned Cylinder | `createCylinder` | creates a cylinder aligned with the specified axis (`x`, `y`, or `z`) at the specified center location (`x0`, `x1`), where (`x0`, `x1`) is (`y`, `z`) for x-axis, (`x`, `z`) for y-axis, (`x`, `y`) for z-axis |

The `CSGSurface` objects can then be accessed or updated with the following methods from `CSGBase`:

- `getAllSurfaces`: retrieve a map of names to shared pointers for of all `CSGSurface` objects
- `getSurfaceByName`: retrieve the shared pointer to the `CSGSurface` of the specified name
- `renameSurface`: change the name of the `CSGSurface`

Examples:

```cpp
// create a plane defined by the points (1, 2, 3), (1, 1, 0), and (0, 0, 0)
auto p1 = Point(1, 2, 3);
auto p2 = Point(1, 1, 0);
auto p3 = Point(0, 0, 0);
auto plane = csg_object->createPlane('new_plane', p1, p2, p3);
```

```cpp
// create a plane at x=2 (a=1, b=c=0, d=2)
auto xplane = csg_obj->createPlane('xplane', 1.0, 0.0, 0.0, 2.0);
```

```cpp
// create a sphere at the origin with radius 5
auto sphere1 = csg_obj->createSphere('origin_sphere', 5.0);
```

```cpp
// create a plane at the point (1, 2, 3) of radius 4
auto center = Point(1, 2, 3);
auto sphere2 = csg_obj->createSphere('new_sphere', center, 4.0);
```

```cpp
// create x-, y-, and z- aligned cylinders of radius 5, each centered at (1, 2, 3)
auto xcyl = csg_obj->createCylinder('xcylinder', 2, 3, 5, 'x');
auto ycyl = csg_obj->createCylinder('ycylinder', 1, 3, 5, 'y');
auto zcyl = csg_obj->createCylinder('zcylinder', 1, 2, 5, 'z');
```

## Cell Methods

A cell is an object defined by a region and a fill.
To create any `CSGCell`, use the method `createCell` from `CSGBase` which will return a shared pointer to the `CSGCell` object that is created.
At the time of calling `createCell`, a unique cell name, the cell region (`CSGRegion`), and an indicator of the fill must be provided.
The `CSGRegion` is defined by boolean combinations of `CSGSurfaces` as described below.
Three types of cell fills are currently supported: void, material, and universe.
If creating a void cell, no fill has to be passed to the creation method.
To create a cell with a material fill, simply provide it with a name of a material as a string.
For a cell with a `CSGUniverse` fill, pass it a shared pointer to the `CSGUniverse`.

The `CSGRegion` objects can then be accessed or updated with the following methods from `CSGBase`:

- `getAllCells`: retrieve a map of names to shared pointers for of all `CSGCell` objects
- `getCellByName`: retrieve the shared pointer to the `CSGCell` of the specified name
- `renameCell`: change the name of the `CSGCell`
- `updateCellRegion`: change the region of the cell; if used, all `CSGSurface` objects used to define the new `CSGRegion` must also be a part of the current `CSGBase`

### Region Definition

As stated, the region for a cell is defined as boolean combinations of surfaces, more specifically the halfspace regions defined by the surfaces.
Series of operations can be defined using parentheses `(` `)` to indicate which operations to perform first.
The types of operators available to define a `CSGRegion` using `CSGSurface` objects are:

| Operator | Description        | Example Use           |
|----------|--------------------|-----------------------|
| `+`      | positive halfspace | `+surf`               |
| `-`      | negative halfspace | `-surf`               |
| `&`      | intersection       | `-surfA & +surfB`     |
| `|`      | union              | `-surfA` `|` `+surfB` |
| `~`      | complement         | `~(-surfA & +surfB)`  |

Here is an example of using a combination of all operators to define the space outside a cylinder of a finite height that is topped with a half-sphere:

`~((-cylinder_surf & -top_plane & +bottom_plane) | (+top_plane & -sphere_surf))`

## Universe Methods

A universe is a collection of cells and is created by calling `createUniverse` from `CSGBase`.
A `CSGUniverse` can be initialized as an empty universe, or by passing a vector of shared pointers to `CSGCell` objects.
Any `CSGUniverse` object can be renamed (including the [root universe](#root-universe)) with `renameUniverse`.

Example:

```cpp
// create an empty universe which will get cells added to it later
auto empty_universe = csg_obj->createUniverse("empty_universe");
// create a universe that is initialized with an existing list of cells
auto new_universe = csg_obj->createUniverse("new_universe", list_of_cells);
```

### Root Universe

In theory, all universes in a model can be traced back to a singular overarching universe known as the root universe.
Because universes are a collection of cells and cells can be filled with universe, a tree of universes can be constructed such that the root universe is the collection of all cells in the model.
When a `CSGBase` object is first initialized, a root `CSGUniverse` called `ROOT_UNIVERSE` is created by default.
Every `CSGCell` that is created will be added to the root universe unless otherwise specified (as described [below](#adding-or-removing-cells)).
The root universe exists by default and cannot be changed except when joining `CSGBase` objects, as described [below](#updating-existing-csgbase-objects).
However, the name of the root universe can be updated, though it won't change the object and its contents directly.

Methods available for managing the root universe:

- `getRootUniverse`: returns a shared pointer to the root universe of the `CSGBase` instance
- `renameRootUniverse`: change the name of the root universe

### Adding or Removing Cells

There are multiple ways in which cells can be added to a universe:

1. At the time of universe creation, a list of pointers to `CSGCell` objects can be passed into `createUniverse` (as described [above](#universe-methods)). Example:

```cpp
auto new_universe = csg_obj->createUniverse("new_universe", list_of_cells);
```

2. When a `CSGCell` is created with `createCell`, a `CSGUniverse` can be passed as the final argument to indicate that the cell will be created and added directly to that specified universe. In this case, the cell will _not_ be added to the root universe. A cell that has a universe fill type cannot be added to the same universe that is being used for the fill. Example:

```cpp
// create an empty universe
auto new_universe = csg_obj->createUniverse("new_univ");
// create a new void cell and add it directly to the new empty universe;
// do not add to the root universe
auto new_cell_in_univ = csg_obj->createCell("new_cell", region, new_universe);
```

3. A cell or list of cells can be added to an existing universe with the `addCellToUniverse` and `addCellsToUniverse` methods. In this case, if a `CSGCell` exists in another `CSGUniverse` (such as the root universe), it will _not_ be removed when being added to another (i.e. if the same behavior as option 2 above is desired, the cell will have to be manually removed from the root universe). The following two examples will produce the same outcome:

```cpp
// create a list of cells and add to an existing universe after creating all of them
std::vector<std::shared_ptr<CSG::CSGCell>> list_of_cells;
for (unsigned int i = 0; i < x; ++i)
{
    // creating new_cell here will add it to the root universe
    auto new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    list_of_cells.pushback(new_cell);
}
// add to an existing universe; cells will still remain in the root universe
csg_obj->addCellsToUniverse(existing_universe, list_of_cells);
```

```cpp
// create new cells and add them to an existing universe one-by-one
// each cell will still exist in the root universe
for (unsigned int i = 0; i < x; ++i)
{
    // creating new_cell here will add it to the root universe
    auto new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    csg_obj->addCellToUniverse(existing_universe, new_cell);
}
```

Cells can also be removed from a universe in the same way as method 3 above by using the `removeCellFromUniverse` and `removeCellsFromUniverse` methods.
An example use would be to take the previous two examples and remove the cells from the root universe such that the desired outcome is the same as that of method 2 for adding cells to a universe:

```cpp
// create a list of cells and add to an existing universe after creating all of them
std::vector<std::shared_ptr<CSG::CSGCell>> list_of_cells;
for (unsigned int i = 0; i < x; ++i)
{
    // creating new_cell here will add it to the root universe
    auto new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    list_of_cells.pushback(new_cell);
}
// add to an existing universe and explicitly remove them from root
csg_obj->addCellsToUniverse(existing_universe, list_of_cells);
csg_obj->removeCellsFromUniverse(csg_obj->getRootUniverse(), list_of_cells);
```

```cpp
// create new cells and add them to an existing universe and remove them from root one-by-one
for (unsigned int i = 0; i < x; ++i)
{
    // creating new_cell here will add it to the root universe
    auto new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    csg_obj->addCellToUniverse(existing_universe, new_cell);
    csg_obj->removeCellFromUniverse(csg_obj->getRootUniverse(), new_cell);
}
```

*Note: When adding and removing cells to/from universes, it is important to maintain the connectedness of all universes meaning all universes should be able to be traced back to the root universe at the end, in order to have a consistent model at the end.*

## Updating Existing CSGBase Objects

An empty `CSGBase` object can be [initialized](#initialization) on its own in each `generateCSG` method for each mesh generator.
However, in most cases, it is necessary to update an existing `CSGBase` object from a previous `MeshGenerator` or join multiple together such that only one `CSGBase` object is ultimately produced at the end of the full generation process.
There are two main ways to handle this: passing and joining.

### Passing between Mesh Generators

The `getCSGBase*` methods available for all [mesh generators](src/meshgenerators/MeshGenerator.md) can be used to access the `CSGBase` object associated with a different `MeshGenerator` and move it to be the current object. Example:

```cpp
// get the CSGBase from a different mesh generator and use in this mesh generator
auto csg_base = getCSGBaseByName(other_mg_name);
std::unique_ptr<CSG::CSGBase> csg_obj = std::move(*csg_base);
// csg_obj is now the object that will continue to get updated throughout
// the generateCSG method.
```

### Joining Bases

When two or more existing `CSGBase` objects need to be combined to continue to use and update, the `joinOtherBase` method should be used.
This method is called from another `CSGBase` and at a minimum takes a different existing `CSGBase` object as input.
There are 3 different behaviors for joining bases that are supported depending on the additional arguments that are passed:

1. No additional arguments: All cells that are in the root universe of the incoming `CSGBase` object will be added to the existing root universe of the current base object, and the root universe from the incoming base will no longer exist.

```cpp
// get a list of bases associated with a list of mesh generator names
const auto csg_bases = getCSGBases("input_mg");
// first mesh generator base will be used as the main and all others will be joined to this one.
std::unique_ptr<CSG::CSGBase> csg_obj = std::move(*csg_bases[0]);
for (unsigned int i = 1; i < csg_bases.size(); ++i)
{
  // get the specific base object from the input and join to the first
  std::unique_ptr<CSG::CSGBase> inp_csg_obj = std::move(*csg_bases[i]);
  // all cells in root for inp_csg_obj are moved to the root of the current csg_obj
  csg_obj->joinOtherBase(inp_csg_obj);
}
```

2. One new root universe name (`new_root_name_join`): All cells in the root universe of the incoming base will be used to create a new universe of the name specified by the `new_root_name_join` parameter. These cells will _not_ be added to the existing root universe. This new universe will be added as a new non-root universe in the existing base object. *Note: this newly created universe will not be connected to the root universe of the existing `CSGBase` object by default.*

```cpp
// get a list of bases associated with a list of mesh generator names
const auto csg_bases = getCSGBasesByName(input_mg_names);
// first mesh generator base will be used as the main
// all others will be joined to this one creating a new universe based on the root of the incoming base.
std::unique_ptr<CSG::CSGBase> csg_obj = std::move(*csg_bases[0]);
for (unsigned int i = 1; i < input_mg_names.size(); ++i)
{
  // get the CSGBase obj for the associated incoming MG
  inp_csg_obj = std::move(*csg_bases[i]);
  // specify a new name for the universe to be created from the incoming root universe
  new_join_name = input_mg_names[i] + "_univ";
  // the root universe of csg_obj will remain unchanged
  // the root universe of inp_csg_obj will be renamed and maintained as a new separate universe
  csg_obj->joinOtherBase(inp_csg_obj, new_join_name);
}
```

3. Two new root universe names (`new_root_name_base` and `new_root_name_join`): The cells in the root universe of the current `CSGBase` object will be used to create a new non-root universe of the name specified by the `new_root_name_base` parameter, and the cells in the root universe of the incoming `CSGBase` object will be used to create a separate non-root universe of the name specified by the `new_root_name_join` parameter. *Note: At the end of this join method, the root universe of the current base object will be empty and neither of the two new non-root universes will be connected to the root universe by default.*

```cpp
// get a list of bases associated with a list of mesh generator names
auto csg_bases = getCSGBasesByName(input_mg_names);
// get the first two bases to join together
std::unique_ptr<CSG::CSGBase> csg_obj = std::move(*csg_bases[0]);
std::unique_ptr<CSG::CSGBase> inp_csg_obj = std::move(*csg_bases[1]);
// specify new names to be used for the new universes created from the root universes of the two bases
std::string new_join_name = input_mg_names[1] + "_univ";
std::string new_base_name = input_mg_names[0] + "_univ";
// joining via this method will move both roots into new universes with new names
csg_obj->joinOtherBase(inp_csg_obj, new_base_name, new_join_name);
```

For all of these join methods, any non-root universes will remain unchanged and simply added to the list of universes for the current `CSGBase` object.
Similarly, all incoming cells and surfaces are added alongside existing cells and surfaces.

It is very important when using the `joinOtherBase` method that all surfaces, cells, and universes are uniquely named.
No two objects of the same type may have the same name.
An error will be produced during the join process if an object of the same name already exists.
See [recommendations for naming](#object-naming-recommendations).

## Accessing CSG Methods

All CSG methods related to creating or changing a CSG object must be called through `CSGBase`.
Calls that retrieve information only but do not manipulate an object (such as `getName` methods) can be called on the object directly.
This ensures proper accounting of all CSG-related objects in the `CSGBase` instance.
Consult the Doxygen documentation for information on all object-specific methods.

## Object Naming Recommendations

For each new CSG element (`CSGSurface`, `CSGCell`, and `CSGUniverse`) that is created, a unique name identifier (of type `std::string`) must be provided (`name` parameter for all creation methods).
A recommended best practice is to include the mesh generator name (which can be accessed with `this->getName()` in any MeshGenerator class) as a part of that object name.
This `name` is used as the unique identifier within the `CSGBase` instance.
Methods for renaming objects are available to help prevent issues and errors.

## CSG Output

describe the universe linking check