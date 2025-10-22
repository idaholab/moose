# CSGBase

`CSGBase` is the main class developers should interact with when implementing the `generateCSG` method for any mesh generator.
This framework class acts as a container and driver for all methods necessary for creating a [constructive solid geometry (CSG)](syntax/CSG/index.md) representation such as generating surfaces, cells, and universes of the mesh generator under consideration.

!alert! note

Throughout this documentation, `csg_obj` will be used in example code blocks to refer to a `CSGBase` instance.

!alert-end!

## Declaring that a mesh generator supports the generation of CSG

In order to call `generateCSG`, the `setHasGenerateCSG` method must be called on the mesh generator to declare that the method has been implemented.

!listing TestCSGAxialSurfaceMeshGenerator.C start=InputParameters end=} include-end=true

## How to implement the `generateCSG` routine

This section will describe the various components developers should implement into the `generateCSG` method for a given [source/meshgenerators/MeshGenerator.md].
This method will return a unique pointer to the `CSGBase` object that was created or modified by the mesh generator in the `generateCSG` method.

### Initialization

A new `CSGBase` object can be initialized with:

!listing ExampleCSGInfiniteSquareMeshGenerator.C start=csg_obj end=csg_obj include-end=true

Once initialized, surfaces, cells, and universes can be created and manipulated.
The following sections explain in detail how to do this as a part of the `generateCSG` method.

### Surfaces

Surfaces are used to define the spatial extent of the region of a `CSGCell`.
To create a `CSGSurface` object, the surface constructor must be called directly to create a unique pointer.
This pointer then has to be passed to the current `CSGBase` instance with `addSurface` which will then return a const reference to that generated surface (`const & CSGSurface`).
The syntax to do this is as follows, where `SurfaceType` should be replaced with the specific type of surface being created (e.g., `CSG::CSGPlane`):

```cpp
// the unique surface pointer is made first, creating the surface object
std::unique_ptr<CSG::CSGSurface> surf_ptr = std::make_unique<SurfaceType>(arguments);
// and then it is explicitly passed to this CSGBase instance, which holds the memory ownership for the object
const auto & surface = csg_obj->addSurface(std::move(surf_ptr));
```

!alert! note title=Adding surfaces to the CSGBase instance

Surfaces need to be added to the CSGBase instance with `addSurface` as described above. If this is not done and these surfaces are referenced in regions used to define cells within the CSGBase instance, an error will occur.

!alert-end!

The `CSG` framework in MOOSE provides various classes for creating basic surfaces (see table below).
Information about how to define new types of surfaces can be found in [source/csg/CSGSurface.md].

| Surface Type | Class | Description |
|---------|--------|------------|
| Plane | `CSGPlane` | create a plane defined by 3 points or from coefficients `a`, `b`, `c`, and `d` for the equation `ax + by + cz = d` |
| Sphere | `CSGSphere` | creates a sphere of radius `r` at an optionally specified center point (default is `(0, 0, 0)`) |
| Cylinder | `CSGXCylinder` | creates a cylinder aligned with the x-axis at the specified center location (`y`, `z`) |
| Cylinder | `CSGYCylinder` | creates a cylinder aligned with the y-axis at the specified center location (`x`, `z`) |
| Cylinder | `CSGZCylinder` | creates a cylinder aligned with the z-axis at the specified center location (`x`, `y`) |

Example:

!listing TestCSGAxialSurfaceMeshGenerator.C start=create end=csg_plane include-end=true

!alert! note title=Including Surface Types

In order to define a surface, the header file for that surface type must be included in the `MeshGenerator.C` file (i.e., `#include "CSGPlane.h"` to create planes).

!alert-end!

The `CSGSurface` objects can then be accessed or updated with the following methods from `CSGBase`:

- `addSurface`: add a unique pointer to a `CSGSurface` object to this `CSGBase` instance
- `getAllSurfaces`: retrieve a list of const references to each `CSGSurface` object in the `CSGBase` instance
- `getSurfaceByName`: retrieve a const reference to the `CSGSurface` of the specified name
- `renameSurface`: change the name of the `CSGSurface`

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
Some examples of creating the different types of cells are shown below:

!listing CSGBaseTest.C start=create a void cell with name cname1 and defined by region reg1 end=csg_obj include-end=true

!listing CSGBaseTest.C start=create a material-filled cell end=csg_obj include-end=true

!listing CSGBaseTest.C start=create a universe-filled cell end=csg_obj include-end=true

!alert! note title=Materials as Placeholders

A cell with a material fill is *not* connected to a MOOSE material definition at this time.
The "material" is currently just a string to represent the name of a CSG material or other type of fill that is otherwise undefined.

!alert-end!

The `CSGCell` objects can then be accessed or updated with the following methods from `CSGBase`:

- `getAllCells`: retrieve a list of const references to each `CSGCell` object in the `CSGBase` instance
- `getCellByName`: retrieve a const reference to the `CSGCell` object of the specified name
- `renameCell`: change the name of the `CSGCell` object
- `updateCellRegion`: change the region of the cell; if used, all `CSGSurface` objects used to define the new `CSGRegion` must also be a part of the current `CSGBase`

### Universes

A universe is a collection of cells and is created by calling `createUniverse` from `CSGBase` which will return a const reference to the `CSGUniverse` object (`const CSGUniverse &`).
A `CSGUniverse` can be initialized as an empty universe, or by passing a vector of shared pointers to `CSGCell` objects.
Any `CSGUniverse` object can be renamed (including the [root universe](#root-universe)) with `renameUniverse`.

The `CSGUniverse` objects can then be accessed or updated with the following methods from `CSGBase`:

- `getAllUniverses`:  retrieve a list of const references to each `CSGUniverse` object in the `CSGBase` instance
- `getUniverseByName`: retrieve a const reference to the `CSGUniverse` of the specified name
- `renameUniverse`: change the name of the `CSGUniverse`

Examples:

!listing CSGBaseTest.C start=new_univ end=new_univ include-end=true

!listing CSGBaseTest.C start=create a list of cells end=createUniverse include-end=True

#### Root Universe

All universes in a model should be able to be traced back, through the hierarchical tree of cells and universes, to a singular overarching universe known as the root universe.
Because universes are a collection of cells and cells can be filled with universe, a tree of universes can be constructed such that the root universe contains the collection of all cells in the model.
When a `CSGBase` object is first [initialized](#initialization), a root `CSGUniverse` called `ROOT_UNIVERSE` is created by default.
Every `CSGCell` that is created will be added to the root universe unless otherwise specified (as described [below](#adding-or-removing-cells)).
The root universe exists by default, and which universe is set as the root cannot be changed, except when joining `CSGBase` objects, as described [below](#updating-existing-csgbase-objects).
However, the name of the root universe can be updated and cells can be manually added or removed using the same methods described [above](#universes).

Methods available for managing the root universe:

- `getRootUniverse`: returns a const reference to the root universe of the `CSGBase` instance
- `renameRootUniverse`: change the name of the root universe

#### Adding or Removing Cells

There are multiple ways in which cells can be added to a universe:

1. At the time of universe creation, a vector of references to `CSGCell` objects can be passed into `createUniverse` (as described [above](#universes)). Example:

!listing CSGBaseTest.C start=create a list of cells to be added to the universe end=createUniverse include-end=true

2. When a `CSGCell` is created with `createCell`, a pointer to a `CSGUniverse` can be passed as the final argument to indicate that the cell will be created and added directly to that specified universe. In this case, the cell will *not* be added to the root universe. A cell that has a universe fill type cannot be added to the same universe that is being used for the fill. For example, the two snippets below come from the same file where a new universe is initialized and passed by reference to the cell when it is created:

!listing CSGBaseTest.C start=make a new universe to which the new cells can be added at time of creation end=add_to_univ include-end=true

!listing CSGBaseTest.C start=create a cell and add to different universe end=createCell include-end=true

3. A cell or list of cells can be added to an existing universe with the `addCellToUniverse` and `addCellsToUniverse` methods. In this case, if a `CSGCell` exists in another `CSGUniverse` (such as the root universe), it will *not* be removed when being added to another (i.e. if the same behavior as option 2 above is desired, the cell will have to be manually removed from the root universe, as described below). The following is an example where the list of cells is collected first and then added at one time to the existing universe, but this could also be accomplished by using `addCellToUniverse` in a for-loop after each cell is initially created.

!listing CSGBaseTest.C start=add a list of cells to an existing universe end=addCellsToUniverse include-end=true

Cells can also be removed from a universe in the same way as method 3 above by using the `removeCellFromUniverse` and `removeCellsFromUniverse` methods.
An example is shown above where the cells are removed from the root universe after they are added to the new universe.
Doing this in multiple steps has the same outcome as that of method 2 for adding cells to a universe at the time of cell creation.

!alert! note title=Maintaining Connectivity

When adding and removing cells to/from universes, it is important to maintain the connectivity of all universes meaning all universes should be nested under the root universe at the end of the generation process, in order to have a consistent model.

!alert-end!

## Updating Existing CSGBase Objects

An empty `CSGBase` object can be [initialized](#initialization) on its own in each `generateCSG` method for each mesh generator.
However, in most cases, it is necessary to update an existing `CSGBase` object from a previous `MeshGenerator` or join multiple `CSGBase` together such that only one `CSGBase` object is ultimately produced at the end of the mesh/CSG generation process.
There are two main ways to handle this: passing and joining.

### Passing between Mesh Generators

`CSGBase` objects from other mesh generators can be accessed through methods that parallel those available for accessing other [source/meshgenerators/MeshGenerator.md] objects.
For all methods listed below, a unique pointer to the `CSGBase` object(s) created by `generateCSG` for the specified [source/meshgenerators/MeshGenerator.md] names are returned.

- `getCSGBase`: get the `CSGBase` object given a parameter name represented as a `std::string` that stores the mesh generator name
- `getCSGBases`: get the `CSGBase` objects given a parameter name represented as a `std::string` that stores a list of mesh generator names
- `getCSGBaseByName`: get the `CSGBase` object given a `MeshGeneratorName`
- `getCSGBasesByName`: get all `CSGBase` objects given a list of `MeshGeneratorName`s

For example:

!listing TestCSGAxialSurfaceMeshGenerator.C start=get the existing CSGBase end=csg_obj include-end=true

!alert! note title=Accessing other MeshGenerator objects by name

A [source/meshgenerators/MeshGenerator.md] object(s) can be passed to another mesh generator as input by providing `InputParameters` of type `MeshGeneratorName`.
See the `ExampleAxialSurfaceMeshGenerator` implementation [below](#example-implementation) for an example of this.

!alert-end!

### Joining Bases

When two or more existing `CSGBase` objects need to be combined to continue to use and update, the `joinOtherBase` method should be used.
This method is called from another `CSGBase` and at a minimum takes a different existing `CSGBase` object as input.
There are 3 different behaviors for joining bases that are supported depending on the additional arguments that are passed:

1. No additional arguments: All cells that are in the root universe of the incoming `CSGBase` object will be added to the existing root universe of the current base object, and the root universe from the incoming base will no longer exist.

!listing CSGBaseTest.C start=Case 1 end=joinOtherBase include-end=true

2. One new root universe name (`new_root_name_join`): All cells in the root universe of the incoming base will be used to create a new universe of the name specified by the `new_root_name_join` parameter. These cells will *not* be added to the existing root universe, which will remain unchanged. This new universe will be added as a new non-root universe in the existing base object. *This newly created universe will not be connected to the root universe of the existing `CSGBase` object by default.*

!listing CSGBaseTest.C start=Case 2 end=joinOtherBase include-end=true

3. Two new root universe names (`new_root_name_base` and `new_root_name_join`): The cells in the root universe of the current `CSGBase` object will be used to create a new non-root universe of the name specified by the `new_root_name_base` parameter, and the cells in the root universe of the incoming `CSGBase` object will be used to create a separate non-root universe of the name specified by the `new_root_name_join` parameter. *At the end of this join method, the root universe of the current base object will be empty and neither of the two new non-root universes will be connected to the root universe by default.*

!listing CSGBaseTest.C start=Case 3 end=joinOtherBase include-end=true

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
For example, if a cell were to be created, the current name and region could be retrieved directly from the `CSGCell` object, but if the name or region needed to be changed, that would need to be handled through `CSGBase`.

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

!listing ExampleCSGInfiniteSquareMeshGenerator.C start=InputParameters

The following example builds on the infinite prism example above by taking a `MeshGeneratorName` for an existing `ExampleCSGInfiniteSquareMeshGenerator` as input and adding planes to create a finite rectangular prism.

!listing TestCSGAxialSurfaceMeshGenerator.C start=InputParameters

If the above methods were to be used, the following input would generate the corresponding [!ac](JSON) output below.

Example Input:

!listing csg_only_chained.i

Example Output:

!listing csg_only_chained_out_csg.json

To run the above example, use `--allow-test-objects`:

```shell
./moose_test-opt --allow-test-objects --csg-only -i tests/csg/csg_only_chained.i
```