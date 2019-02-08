//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GridPartitioner.h"

#include "GeneratedMesh.h"
#include "MooseApp.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", GridPartitioner);

#include <memory>

template <>
InputParameters
validParams<GridPartitioner>()
{
  // These two are in this order because they are from different systems
  // so you have to apply _this_ system's second to override the base
  InputParameters params = validParams<GeneratedMesh>();
  params += validParams<MoosePartitioner>();

  // These are suppressed because they're going to get set programmatically
  params.suppressParameter<MooseEnum>("elem_type");
  params.suppressParameter<Real>("xmin");
  params.suppressParameter<Real>("ymin");
  params.suppressParameter<Real>("zmin");
  params.suppressParameter<Real>("xmax");
  params.suppressParameter<Real>("ymax");
  params.suppressParameter<Real>("zmax");
  params.suppressParameter<MooseEnum>("dim");

  params.set<MooseEnum>("parallel_type") = "REPLICATED";
  params.set<MooseEnum>("partitioner") = "LINEAR";

  params.addClassDescription("Create a uniform grid that overlays the mesh to be partitioned.  "
                             "Assign all elements within each cell of the grid to the same "
                             "processor.");

  return params;
}

GridPartitioner::GridPartitioner(const InputParameters & params)
  : MoosePartitioner(params), _mesh(*getCheckedPointerParam<MooseMesh *>("mesh"))
{
}

GridPartitioner::~GridPartitioner() {}

std::unique_ptr<Partitioner>
GridPartitioner::clone() const
{
  return libmesh_make_unique<GridPartitioner>(_pars);
}

void
GridPartitioner::_do_partition(MeshBase & mesh, const unsigned int /*n*/)
{
  // First: build the GeneratedMesh

  auto & factory = _app.getFactory();

  // Figure out the physical bounds of the given mesh
  auto bounding_box = MeshTools::create_bounding_box(mesh);
  const auto & min = bounding_box.min();
  const auto & max = bounding_box.max();

  // Fill up a parameters object from the parameters for this class
  auto params = factory.getValidParams("GeneratedMesh");
  params.applyParameters(_pars);

  auto dim = mesh.mesh_dimension();
  params.set<MooseEnum>("dim") = dim;

  //  Need to make sure the number of cells in the grid matches the number of procs to partition for
  auto num_cells = getParam<unsigned int>("nx");

  if (dim >= 2)
    num_cells *= getParam<unsigned int>("ny");

  if (dim == 3)
    num_cells *= getParam<unsigned int>("nz");

  if (num_cells != mesh.n_partitions())
    mooseError("Number of cells in the GridPartitioner must match the number of MPI ranks! ",
               num_cells,
               " != ",
               mesh.n_partitions());

  params.set<Real>("xmin") = min(0);
  params.set<Real>("ymin") = min(1);
  params.set<Real>("zmin") = min(2);

  params.set<Real>("xmax") = max(0);
  params.set<Real>("ymax") = max(1);
  params.set<Real>("zmax") = max(2);

  auto grid_mesh_ptr = _app.getFactory().create<MooseMesh>("GeneratedMesh", name() + "_gm", params);
  grid_mesh_ptr->init();

  auto point_locator_ptr = grid_mesh_ptr->getPointLocator();

  // Loop over all of the elements in the given mesh
  for (auto & elem_ptr : mesh.active_element_ptr_range())
  {
    // Find the element it lands in in the GeneratedMesh
    auto grid_elem_ptr = (*point_locator_ptr)(elem_ptr->centroid());

    // True if we found something
    if (grid_elem_ptr)
      // Assign the _id_ of the cell to the processor_id
      elem_ptr->processor_id() = grid_elem_ptr->id();
    else // Should never happen (seriously - we create bounding boxes that should disallow this!
      mooseError("GridPartitioner unable to locate element within the grid!");
  }
}
