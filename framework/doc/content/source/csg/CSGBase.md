# CSGBase

`CSGBase` is the main class developers should interact with when implementing the `generateCSG` method for any mesh generator.
This framework class acts as a container and driver for all methods necessary for creating a [constructive solid geometry (CSG)](source/csg/CSG.md) representation such as generating surfaces, cells, and universes of the mesh generator under consideration.

!alert! note

Throughout this documentation, `csg_obj` will be used in example code blocks to refer to a `CSGBase` instance.

!alert-end!

## Declaring that a mesh generator supports the generation of CSG

In order to call `generateCSG`, the `setHasGenerateCSG` method must be called on the mesh generator to declare that the method has been implemented.

```cpp
#include "CSGBase.h"

InputParameters
ExampleMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // Example input parameter that is an existing mesh generator
  params.addRequiredParam<MeshGeneratorName>("input_mg", "The input MeshGenerator.");
  ...
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}
```

## How to implement the `generateCSG` routine

This section will describe the various components developers should implement into the `generateCSG` method for a given [source/meshgenerators/MeshGenerator.md].
This method will return a unique pointer to the `CSGBase` object that was created or modified by the mesh generator in the `generateCSG` method.

### Initialization

A new `CSGBase` object can be initialized with:

```cpp
auto csg_obj = std::make_unique<CSG::CSGBase>();
```

Once initialized, surfaces, cells, and universes can be created and manipulated.
The following sections explain in detail how to do this as a part of the `generateCSG` method.

### Surfaces

Surfaces are used to define the spatial extent of the region of a `CSGCell`.
Various methods exist to create `CSGSurface` objects (below).
All surface creation methods will return a const reference to that generated surface (`const & CSGSurface`).

| Surface Type | Method | Description |
|---------|--------|------------|
| Plane | `createPlaneFromPoints` | create a plane defined by 3 points |
| Plane | `createPlaneFromCoefficients` | creates a plane based on the coefficients `a`, `b`, `c`, and `d` for the equation `ax + by + cz = d` |
| Sphere | `createSphere` | creates a sphere of radius `r` at an optionally specified center point (default is `(0, 0, 0)`) |
| Axis-Aligned Cylinder | `createCylinder` | creates a cylinder aligned with the specified axis (`x`, `y`, or `z`) at the specified center location (`x0`, `x1`), where (`x0`, `x1`) is (`y`, `z`) for X-Cylinder, (`x`, `z`) for Y-Cylinder, and (`x`, `y`) for Z-Cylinder |

At the time of surface creation, the type of boundary (`CSGSurface::BoundaryType`) can be optionally set.
Options for boundary types are `TRANSMISSION` (default), `VACUUM`, and `REFLECTIVE`.

The `CSGSurface` objects can then be accessed or updated with the following methods from `CSGBase`:

- `getAllSurfaces`: retrieve a list of const references to all `CSGSurface` objects defined within the `CSGBase` object
- `getSurfaceByName`: retrieve a const reference to the `CSGSurface` of the specified name
- `renameSurface`: change the name of the `CSGSurface`
- `updateSurfaceBoundaryType`: change the boundary type of the `CSGSurface` (`TRANSMISSION`, `VACUUM`, or `REFLECTIVE`)

Examples:

```cpp
// create a plane defined by the points (1, 2, 3), (1, 1, 0), and (0, 0, 0)
// set a reflective boundary type at the time of creation
auto p1 = Point(1, 2, 3);
auto p2 = Point(1, 1, 0);
auto p3 = Point(0, 0, 0);
auto bc_refl = CSG::CSGSurface::BoundaryType::REFLECTIVE;
const auto & plane = csg_object->createPlane('new_plane', p1, p2, p3, bc_refl);
```

```cpp
// create a plane at x=2 (a=1, b=c=0, d=2)
const auto & xplane = csg_obj->createPlane('xplane', 1.0, 0.0, 0.0, 2.0);
```

```cpp
// create a sphere at the origin with radius 5
const auto & sphere1 = csg_obj->createSphere('origin_sphere', 5.0);
```

```cpp
// create a sphere at the point (1, 2, 3) of radius 4
auto center = Point(1, 2, 3);
const auto & sphere2 = csg_obj->createSphere('new_sphere', center, 4.0);
```

```cpp
// create x-, y-, and z- aligned cylinders of radius 5, each centered at (1, 2, 3)
const auto & xcyl = csg_obj->createCylinder('xcylinder', 2, 3, 5, 'x');
const auto & ycyl = csg_obj->createCylinder('ycylinder', 1, 3, 5, 'y');
const auto & zcyl = csg_obj->createCylinder('zcylinder', 1, 2, 5, 'z');
```

### Regions

A region is a space defined by boolean operations applied to surfaces and other regions.
Half-space regions are defined as the positive and negative space separated by a surface.
These regions can be unionized, intersected, or the complement taken to further define more complex regions.
Series of operations can be defined using parentheses `(` `)` to indicate which operations to perform first.
The types of operators available to define a `CSGRegion` using `CSGSurface` objects are:

| Operator | Description        | Example Use           |
|----------|--------------------|-----------------------|
| `+`      | positive half-space | `+surf`               |
| `-`      | negative half-space | `-surf`               |
| `&`      | intersection       | `-surfA & +surfB`     |
| `|`      | union              | `-surfA` `|` `+surfB` |
| `~`      | complement         | `~(-surfA & +surfB)`  |
| `&=`     | update existing region with an intersection | `region1 &= -surfA` |
| `|``=`   | update existing region with a union | `region1` `|``= +surfB` |

The following is an example of using a combination of all operators to define the space outside a cylinder of a finite height that is topped with a half-sphere.
Each of the half-spaces associated with each surface are shown in [!ref](fig:region_surfs).
The cylinder and planes are then combined via intersection to form the region inside a finite cylinder, and the space above the top plane is intersected with the sphere to define a half sphere ([!ref](fig:region1)).
These two regions are unionized as shown in [!ref](fig:region2).
The complement of the previous combination then defines the final region `~((-cylinder_surf & -top_plane & +bottom_plane) | (+top_plane & -sphere_surf))`, as shown in blue in [!ref](fig:region3).

!media large_media/csg/region_surfs.png
       id=fig:region_surfs
       caption=Four different surfaces: an infinite cylinder (blue), a top plane (orange), a bottom plane (red), and a sphere (green)

!media large_media/csg/region1.png
       id=fig:region1
       caption=Two separate regions both defined as *intersections* of *half-spaces*.

!media large_media/csg/region2.png
       id=fig:region2
       caption=One region defined by the *union* of two other regions.

!media large_media/csg/region3.png
       id=fig:region3
       caption=A region defined as the *complement* of an existing region.

### Cells

A cell is an object defined by a region and a fill.
To create any `CSGCell`, use the method `createCell` from `CSGBase` which will return a const reference to the `CSGCell` object that is created (`const CSGCell &`).
At the time of calling `createCell`, a unique cell name, the cell region (`CSGRegion`), and an indicator of the fill must be provided.
The `CSGRegion` is defined by boolean combinations of `CSGSurfaces` as described below.
Three types of cell fills are currently supported: void, material, and universe.
If creating a void cell, no fill has to be passed to the creation method.
To create a cell with a material fill, simply provide it with a name of a material as a string.
For a cell with a `CSGUniverse` fill, pass it a shared pointer to the `CSGUniverse`.

The `CSGCell` objects can then be accessed or updated with the following methods from `CSGBase`:

- `getAllCells`: retrieve a list of const references to all `CSGCell` objects defined within the `CSGBase` object
- `getCellByName`: retrieve a const reference to the `CSGCell` object of the specified name
- `renameCell`: change the name of the `CSGCell` object
- `updateCellRegion`: change the region of the cell; if used, all `CSGSurface` objects used to define the new `CSGRegion` must also be a part of the current `CSGBase`

### Universes

A universe is a collection of cells and is created by calling `createUniverse` from `CSGBase` which will return a const reference to the `CSGUniverse` object (`const CSGUniverse &`).
A `CSGUniverse` can be initialized as an empty universe, or by passing a vector of shared pointers to `CSGCell` objects.
Any `CSGUniverse` object can be renamed (including the [root universe](#root-universe)) with `renameUniverse`.

The `CSGUniverse` objects can then be accessed or updated with the following methods from `CSGBase`:

- `getAllUniverses`: retrieve a list of const references to all `CSGUniverse` objects defined within the `CSGBase` object
- `getUniverseByName`: retrieve a const reference to the `CSGUniverse` of the specified name
- `renameUniverse`: change the name of the `CSGUniverse`

Example:

```cpp
// create an empty universe which will get cells added to it later
const auto & empty_universe = csg_obj->createUniverse("empty_universe");
// create a universe that is initialized with an existing list of references to CSGCell objects
const auto & new_universe = csg_obj->createUniverse("new_universe", list_of_cells);
```

#### Root Universe

In theory, all universes in a model can be traced back to a singular overarching universe known as the root universe.
Because universes are a collection of cells and cells can be filled with universe, a tree of universes can be constructed such that the root universe is the collection of all cells in the model.
When a `CSGBase` object is first [initialized](#initialization), a root `CSGUniverse` called `ROOT_UNIVERSE` is created by default.
Every `CSGCell` that is created will be added to the root universe unless otherwise specified (as described [below](#adding-or-removing-cells)).
The root universe exists by default and cannot be changed except when joining `CSGBase` objects, as described [below](#updating-existing-csgbase-objects).
However, the name of the root universe can be updated, though it won't change the object and its contents.

Methods available for managing the root universe:

- `getRootUniverse`: returns a const reference to the root universe of the `CSGBase` instance
- `renameRootUniverse`: change the name of the root universe

#### Adding or Removing Cells

There are multiple ways in which cells can be added to a universe:

1. At the time of universe creation, a list of references to `CSGCell` objects can be passed into `createUniverse` (as described [above](#universes)). Example:

```cpp
const auto & new_universe = csg_obj->createUniverse("new_universe", list_of_cells);
```

2. When a `CSGCell` is created with `createCell`, a pointer to a `CSGUniverse` can be passed as the final argument to indicate that the cell will be created and added directly to that specified universe. In this case, the cell will *not* be added to the root universe. A cell that has a universe fill type cannot be added to the same universe that is being used for the fill. For example:

```cpp
// create an empty universe
const auto & new_universe = csg_obj->createUniverse("new_univ");
// create a new void cell and add it directly to the new empty universe;
// do not add to the root universe
const auto & new_cell_in_univ = csg_obj->createCell("new_cell", region, &new_universe);
```

3. A cell or list of cells can be added to an existing universe with the `addCellToUniverse` and `addCellsToUniverse` methods. In this case, if a `CSGCell` exists in another `CSGUniverse` (such as the root universe), it will *not* be removed when being added to another (i.e. if the same behavior as option 2 above is desired, the cell will have to be manually removed from the root universe). The following two examples will produce the same outcome:

```cpp
// create a list of cells and add to an existing universe after creating all of them
std::vector<std::reference_wrapper<const CSG::CSGCell>> list_of_cells;
for (unsigned int i = 0; i < num_cells_to_add; ++i)
{
    // creating new_cell here will add it to the root universe
    const auto & new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    list_of_cells.push_back(new_cell);
}
// add to an existing universe; cells will still remain in the root universe
csg_obj->addCellsToUniverse(existing_universe, list_of_cells);
```

```cpp
// create new cells and add them to an existing universe one-by-one
// each cell will still exist in the root universe
for (unsigned int i = 0; i < num_cells_to_add; ++i)
{
    // creating new_cell here will add it to the root universe
    const auto & new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    csg_obj->addCellToUniverse(existing_universe, new_cell);
}
```

Cells can also be removed from a universe in the same way as method 3 above by using the `removeCellFromUniverse` and `removeCellsFromUniverse` methods.
An example use would be to take the previous two examples and remove the cells from the root universe such that the desired outcome is the same as that of method 2 for adding cells to a universe:

```cpp
// create a list of cells and add to an existing universe after creating all of them
std::vector<std::reference_wrapper<const CSG::CSGCell>> list_of_cells;
for (unsigned int i = 0; i < num_cells_to_add; ++i)
{
    // creating new_cell here will add it to the root universe
    const auto & new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    list_of_cells.push_back(new_cell);
}
// add to an existing universe and explicitly remove them from root
csg_obj->addCellsToUniverse(existing_universe, list_of_cells);
csg_obj->removeCellsFromUniverse(csg_obj->getRootUniverse(), list_of_cells);
```

```cpp
// create new cells and add them to an existing universe and remove them from root one-by-one
for (unsigned int i = 0; i < num_cells_to_add; ++i)
{
    // creating new_cell here will add it to the root universe
    const auto & new_cell = csg_obj->createCell("new_cell_" + std::to_string(i), regions[i]);
    csg_obj->addCellToUniverse(existing_universe, new_cell);
    csg_obj->removeCellFromUniverse(csg_obj->getRootUniverse(), new_cell);
}
```

!alert! note title=Maintaining Connectivity

When adding and removing cells to/from universes, it is important to maintain the connectivity of all universes meaning all universes should be able to be traced back to the root universe at the end of the generation process, in order to have a consistent model.

!alert-end!

## Updating Existing CSGBase Objects

An empty `CSGBase` object can be [initialized](#initialization) on its own in each `generateCSG` method for each mesh generator.
However, in most cases, it is necessary to update an existing `CSGBase` object from a previous `MeshGenerator` or join multiple together such that only one `CSGBase` object is ultimately produced at the end of the full generation process.
There are two main ways to handle this: passing and joining.

### Passing between Mesh Generators

`CSGBase` objects from other mesh generators can be accessed through methods that parallel those available for accessing other [source/meshgenerators/MeshGenerator.md] objects.
For all methods listed below, a unique pointer to the `CSGBase` object(s) created by `generateCSG` for the specified [source/meshgenerators/MeshGenerator.md] names are returned.

- `getCSGBase`: get the `CSGBase` object given a parameter name represented as a `std::string` that stores the mesh generator name
- `getCSGBases`: get the `CSGBase` objects given a parameter name represented as a `std::string` that stores a list of mesh generator names
- `getCSGBaseByName`: get the `CSGBase` object given a `MeshGeneratorName`
- `getCSGBasesByName`: get all `CSGBase` objects given a list of `MeshGeneratorName`s

For example:

```cpp
// get the CSGBase from a different mesh generator and use in this mesh generator
// other_mg_name is a MeshGeneratorName object
auto csg_base = getCSGBaseByName(other_mg_name);
std::unique_ptr<CSG::CSGBase> csg_obj = std::move(*csg_base);
// csg_obj is now the object that will continue to get updated throughout the generateCSG method.
```

!alert! note title=Accessing other MeshGenerator objects by name

A [source/meshgenerators/MeshGenerator.md] object(s) can be passed to another mesh generator as input by providing `InputParameters` of type `MeshGeneratorName`.
See the `ExampleAxialSurfaceMeshGenerator` implementation [below](#example-implementation) for an example of this.

!alert-end!

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

2. One new root universe name (`new_root_name_join`): All cells in the root universe of the incoming base will be used to create a new universe of the name specified by the `new_root_name_join` parameter. These cells will *not* be added to the existing root universe. This new universe will be added as a new non-root universe in the existing base object. *This newly created universe will not be connected to the root universe of the existing `CSGBase` object by default.*

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

3. Two new root universe names (`new_root_name_base` and `new_root_name_join`): The cells in the root universe of the current `CSGBase` object will be used to create a new non-root universe of the name specified by the `new_root_name_base` parameter, and the cells in the root universe of the incoming `CSGBase` object will be used to create a separate non-root universe of the name specified by the `new_root_name_join` parameter. *At the end of this join method, the root universe of the current base object will be empty and neither of the two new non-root universes will be connected to the root universe by default.*

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

!alert! note title=Object Naming Uniqueness

It is very important when using the `joinOtherBase` method that all `CSGSurfaces`, `CSGCells`, and `CSGSurfaces` are uniquely named so that errors are not encountered when combining sets of objects.
An error will be produced during the join process if an object of the same type and name already exists.
See [recommendations for naming](#object-naming-recommendations) below.

!alert-end!

## Accessing CSG-related Methods

All [!ac](CSG) methods related to creating or changing a [!ac](CSG) object must be called through `CSGBase`.
Calls that retrieve information only but do not manipulate an object (such as `getName` methods) can be called on the object directly.
For example, if a cell were to be created, the current name and region could be retrieved directly from the `CSGCell` object, but if the name or region needed to be changed, that would need to be handled through `CSGBase`:

```cpp
// a cell is created using CSGBase
const auto & cell = csg_obj->createCell("cell_name", region);
// the current name and region of that cell can be retrieved directly from the CSGCell object
const auto name = cell.getName();
const auto & region = cell.getRegion();

// changing the name and region requires using methods in CSGBase
csg_obj->renameCell(cell, "new_name");
csg_obj->updateCellRegion(cell, new_region);
```

This ensures proper accounting of all [!ac](CSG)-related objects in the `CSGBase` instance.
Consult the Doxygen documentation for information on all object-specific methods.

## Object Naming Recommendations

For each new [!ac](CSG) element (`CSGSurface`, `CSGCell`, and `CSGUniverse`) that is created, a unique name identifier (of type `std::string`) must be provided (`name` parameter for all creation methods).
A recommended best practice is to include the mesh generator name (which can be accessed with `this->getName()` in any MeshGenerator class) as a part of that object name.
This `name` is used as the unique identifier within the `CSGBase` instance.
Methods for renaming objects are available as described in the above sections to help prevent issues and errors.

## Example Implementation

Provided here is an example implementation of the `generateCSG` method for a simple example [source/meshgenerators/MeshGenerator.md] that creates an infinite rectangular prism given an input parameter for `side_length`.
The code snippets provided here correspond to the `.C` file.

```cpp
#include "CSGBase.h"

InputParameters
ExamplePrismCSGMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<Real>("side_length", "Side length of infinite prism.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}


std::unique_ptr<CSG::CSGBase>
ExamplePrismCSGMeshGenerator::generateCSG()
{
  // name of the current mesh generator to use for naming generated objects
  auto mg_name = this->getName();

  // initialize a CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // sets of points to use to define the 4 surfaces of the infinite prism
  std::vector<std::vector<Point>> points_on_planes{{Point(1. * _side_length / 2., 0., 0.),
                                                    Point(1. * _side_length / 2., 1., 0.),
                                                    Point(1. * _side_length / 2., 0., 1.)},
                                                   {Point(-1. * _side_length / 2., 0., 0.),
                                                    Point(-1. * _side_length / 2., 1., 0.),
                                                    Point(-1. * _side_length / 2., 0., 1.)},
                                                   {Point(0., 1. * _side_length / 2., 0.),
                                                    Point(1., 1. * _side_length / 2., 0.),
                                                    Point(0., 1. * _side_length / 2., 1.)},
                                                   {Point(0., -1. * _side_length / 2., 0.),
                                                    Point(1., -1. * _side_length / 2., 0.),
                                                    Point(0., -1. * _side_length / 2., 1.)}};
  std::vector<std::string> surf_names{"plus_x", "minus_x", "plus_y", "minus_y"};

  // initialize cell region to be updated
  CSG::CSGRegion region;

  // set the center of the prism to be used for determining half-spaces
  const auto centroid = Point(0, 0, 0);

  // create each plane and update the region to be used for the cell as each new plane is created
  for (unsigned int i = 0; i < points_on_planes.size(); ++i)
  {
    // object name includes the mesh generator name for uniqueness
    const auto surf_name = mg_name + "_surf_" + surf_names[i];
    // create the plane for one face of the prism
    const auto & csg_plane = csg_obj->createPlaneFromPoints(
        surf_name, points_on_planes[i][0], points_on_planes[i][1], points_on_planes[i][2]);
    // determine where the plane is in relation to the centroid to be able to set the half-space
    const auto region_direction = csg_plane.directionFromPoint(centroid);
    // half-space is either positive (+plane_ptr) or negative (-plane_ptr)
    / /depending on the direction to the centroid
    auto halfspace =
        ((region_direction == CSG::CSGSurface::Direction::POSITIVE) ? +csg_plane : -csg_plane);
    // check if this is the first half-space to be added to the region,
    // if not, update the existing region with the intersection of the regions (&=)
    if (region.getRegionType() == CSG::CSGRegion::RegionType::EMPTY)
      region = halfspace;
    else
      region &= halfspace;
  }

  // create the cell defined by the surfaces and region just created
  const auto cell_name = mg_name + "_square_cell";
  const auto material_name = "square_material";
  csg_obj->createCell(cell_name, material_name, region);

  return csg_obj;
}
```

The following example builds on the infinite prism example above by taking a `MeshGeneratorName` for an existing `ExamplePrismCSGMeshGenerator` as input and adding planes to create a finite rectangular prism.

```cpp
InputParameters
ExampleAxialSurfaceMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input MeshGenerator.");
  params.addRequiredParam<Real>("axial_height", "Axial height of output.");
  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  // Declare that this generator has a generateCSG method
  MeshGenerator::setHasGenerateCSG(params);
  return params;
}

std::unique_ptr<CSG::CSGBase>
ExampleAxialSurfaceMeshGenerator::generateCSG()
{
  // get the existing CSGBase associated with the input mesh generator
  // this is the CSGBase object that will be updated
  std::unique_ptr<CSG::CSGBase> csg_obj = std::move(getCSGBase("input"));

  // get the names of the current mesh generator and the input mesh generator
  // so that unique object naming can be enforced
  auto mg_name = this->getName();
  auto inp_name = getParam<MeshGeneratorName>("input");

  // get the expected existing cell
  const auto cell_name = inp_name + "_square_cell";
  const auto & inp_cell = csg_obj->getCellByName(cell_name);
  // get the existing cell region to update
  auto cell_region = inp_cell.getRegion();

  // centroid used to determine direction for half-space
  const auto centroid = Point(0, 0, 0);

  // Add surfaces and half-spaces corresponding to top and bottom axial planes
  std::vector<std::string> surf_names{"plus_z", "minus_z"};
  std::vector<Real> coeffs{0.5 * _axial_height, -0.5 * _axial_height};
  for (unsigned int i = 0; i < coeffs.size(); ++i)
  {
    // unique surface name
    const auto surf_name = mg_name + "_surf_" + surf_names[i];
    // create the plane
    // z plane equation: 0.0*x + 0.0*y + 1.0*z = (+/-)0.5 * axial_height
    const auto & csg_plane = csg_obj->createPlaneFromCoefficients(surf_name, 0.0, 0.0, 1.0, coeffs[i]);
    // determine the half-space to add as an updated intersection
    const auto region_direction = csg_plane.directionFromPoint(centroid);
    auto halfspace =
        ((region_direction == CSG::CSGSurface::Direction::POSITIVE) ? +csg_plane : -csg_plane);
    // update the existing region with a half-space
    cell_region &= halfspace;
  }

  // set the new region for the existing cell
  csg_obj->updateCellRegion(inp_cell, cell_region);

  // rename the root universe which currently contains just the cell defined by inp_cell
  csg_obj->renameRootUniverse(mg_name + "_finite_prism_univ");

  return csg_obj;
}
```

If the above methods were to be used, the following input would generate the corresponding [!ac](JSON) output below.

Example Input:

```
[Mesh]
  [Prism]
    type = ExamplePrismCSGMeshGenerator
    side_length = 4
  []
  [Cube]
    type = ExampleAxialSurfaceMeshGenerator
    input = Prism
    axial_height = 5
  []
[]
```

Example Output:

```json
{
  "CELLS": {
    "Prism_square_cell": {
      "FILL": "square_material",
      "FILLTYPE": "MATERIAL",
      "REGION":
          "(+Prism_surf_plus_x & -Prism_surf_minus_x & -Prism_surf_plus_y & +Prism_surf_minus_y & -Cube_surf_plus_z & +Cube_surf_minus_z)"
    }
  },
  "SURFACES": {
    "Prism_surf_minus_x": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "a": -1.0,
        "b": 0.0,
        "c": 0.0,
        "d": 2.0
      },
      "TYPE": "plane"
    },
    "Prism_surf_minus_y": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "a": 0.0,
        "b": 1.0,
        "c": 0.0,
        "d": -2.0
      },
      "TYPE": "plane"
    },
    "Cube_surf_minus_z": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "a": 0.0,
        "b": 0.0,
        "c": 1.0,
        "d": -2.5
      },
      "TYPE": "plane"
    },
    "Prism_surf_plus_x": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "a": -1.0,
        "b": 0.0,
        "c": 0.0,
        "d": -2.0
      },
      "TYPE": "plane"
    },
    "Prism_surf_plus_y": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "a": 0.0,
        "b": 1.0,
        "c": 0.0,
        "d": 2.0
      },
      "TYPE": "plane"
    },
    "Cube_surf_plus_z": {
      "BOUNDARY": "transmission",
      "COEFFICIENTS": {
        "a": 0.0,
        "b": 0.0,
        "c": 1.0,
        "d": 2.5
      },
      "TYPE": "plane"
    }
  },
  "UNIVERSES": {
    "Prism_finite_prism_univ": {
      "CELLS": [
        "Prism_square_cell"
      ],
      "ROOT": true
    }
  }
}
```
