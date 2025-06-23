# Constructive Solid Geometry

[!ac](CSG) is a geometry representation in which complex models are created through boolean combinations of surfaces, cells, and universes.
[!ac](CSG) models are most commonly used for [!ac](MC) neutronics simulations.
While each [!ac](MC) code has their own syntax for defining the [!ac](CSG) model, the underlying theory for creating [!ac](CSG) representations is the same throughout.
The `CSGBase` class provides the framework in MOOSE for creating these generic [!ac](CSG) representations of mesh generators that can then be used by [!ac](MC) codes.

## Theory

As stated, a [!ac](CSG) representation is defined minimally as a series of surfaces, cells, and universes.
This section describes in theory what these components are.

[Surfaces]((source/csg/CSGBase.md#surfaces)) are defined explicitly through surface equations (such as equations of a plane, sphere, etc.).
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

## For Developers

The `CSGBase` class contains the framework necessary for creating generic [!ac](CSG) definitions, but the methods for actually using it to generate the [!ac](CSG) definition have to be implemented for each mesh generator by overriding the `generateCSG` method according to the information provided [here](source/csg/CSGBase.md).

### setHasGenerateCSG

In order to call `generateCSG`, the `setHaseGenerateCSG` method must be called on the mesh generator to tell it that the method has been implemented.

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

### generateCSG

The `generateCSG` method can then be implemented that will construct surfaces, cells, universes, etc. according to the information provided in [CSGBase](source/csg/CSGBase.md).
This method will return a unique pointer to the `CSGBase` object that was created or modified by this mesh generator.

```cpp
std::unique_ptr<CSG::CSGBase>
ExampleMeshGenerator::generateCSG()
{
  // initialize a new CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  // add in logic to make surface, cells, and universes as appropriate

  return csg_obj;
}
```

### Accessing CSGBase objects

`CSGBase` objects from other mesh generators can be accessed through methods that parallel those available for accessing other [source/meshgenerators/MeshGenerator.md] objects.
For all methods listed below, a unique pointer to the `CSGBase` object(s) created by `generateCSG` for the specified [source/meshgenerators/MeshGenerator.md] names are returned.

- `getCSGBase`: get the `CSGBase` object given a mesh generator name represented as a `std::string`
- `getCSGBases`: get all `CSGBase` objects given a list of mesh generator names represented as `std::string`s
- `getCSGBaseByName`: get the `CSGBase` object given a `MeshGeneratorName`
- `getCSGBasesByName`: get all `CSGBase` objects given a list of `MeshGeneratorName`s

Once a `CSGBase` object is obtained, it can be updated or joined to another existing `CSGBase` object according to the instructions provided [here](source/csg/CSGBase.md#updating-existing-csgbase-objects).

### Example Implementation

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

  // set the center of the prism to be used for determining halfspaces
  const auto centroid = Point(0, 0, 0);

  // create each plane and update the region to be used for the cell as each new plane is created
  for (unsigned int i = 0; i < points_on_planes.size(); ++i)
  {
    // object name includes the mesh generator name for uniqueness
    const auto surf_name = mg_name + "_surf_" + surf_names[i];
    // create the plane for one face of the prism
    auto plane_ptr = csg_obj->createPlaneFromPoints(
        surf_name, points_on_planes[i][0], points_on_planes[i][1], points_on_planes[i][2]);
    // determine where the plane is in relation to the centroid to be able to set the halfspace
    const auto region_direction = plane_ptr->directionFromPoint(centroid);
    // halfspace is either positive (+plane_ptr) or negative (-plane_ptr)
    / /depending on the direction to the centroid
    auto halfspace =
        ((region_direction == CSG::CSGSurface::Direction::POSITIVE) ? +plane_ptr : -plane_ptr);
    // check if this is the first halfspace to be added to the region,
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
  auto cell_ptr = csg_obj->getCellByName(cell_name);
  // get the existing cell region to update
  auto cell_region = cell_ptr->getRegion();

  // centroid used to determine direction for halfspace
  const auto centroid = Point(0, 0, 0);

  // Add surfaces and halfspaces corresponding to top and bottom axial planes
  std::vector<std::string> surf_names{"plus_z", "minus_z"};
  std::vector<Real> coeffs{0.5 * _axial_height, -0.5 * _axial_height};
  for (unsigned int i = 0; i < coeffs.size(); ++i)
  {
    // unique surface name
    const auto surf_name = mg_name + "_surf_" + surf_names[i];

    // create the plane
    // z plane equation: 0.0*x + 0.0*y + 1.0*z = (+/-)0.5 * axial_height
    auto plane_ptr = csg_obj->createPlaneFromCoefficients(surf_name, 0.0, 0.0, 1.0, coeffs[i]);
    // determine the halfspace to add as an updated intersection
    const auto region_direction = plane_ptr->directionFromPoint(centroid);
    auto halfspace =
        ((region_direction == CSG::CSGSurface::Direction::POSITIVE) ? +plane_ptr : -plane_ptr);
    // update the existing region with a halfspace
    cell_region &= halfspace;
  }

  // set the new region for the existing cell
  csg_obj->updateCellRegion(cell_ptr, cell_region);

  // rename the root universe which currently contains just the cell defined by cell_ptr
  csg_obj->renameRootUniverse(mg_name + "_finite_prism");

  return csg_obj;
}
```

If the above methods were to be used, the following input would generate the corresponding [!ac](JSON) input below.

add example input block

add example json output block


### Output

description of the JSON structure and what is provided

