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

InputParameters
GridPartitioner::validParams()
{
  // These two are in this order because they are from different systems
  // so you have to apply _this_ system's second to override the base
  InputParameters params = MoosePartitioner::validParams();

  // Users specify how many processors they need along each direction
  params.addParam<unsigned int>("nx", 1, "Number of processors in the X direction");
  params.addParam<unsigned int>("ny", 1, "Number of processors in the Y direction");
  params.addParam<unsigned int>("nz", 1, "Number of processors in the Z direction");

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
  return std::make_unique<GridPartitioner>(_pars);
}

void
GridPartitioner::_do_partition(MeshBase & mesh, const unsigned int /*n*/)
{
  // By default, there are one processor along each direction
  // nx: the number of processors along x direction
  // ny: the number of processors along y direction
  // nz: the number of processors along z direction
  unsigned int nx = 1, ny = 1, nz = 1;

  // Figure out the physical bounds of the given mesh
  auto bounding_box = MeshTools::create_bounding_box(mesh);
  const auto & min = bounding_box.min();
  const auto & max = bounding_box.max();

  auto dim = mesh.mesh_dimension();
  //  Need to make sure the number of cells in the grid matches the number of procs to partition for
  nx = getParam<unsigned int>("nx");

  if (dim >= 2)
    ny = getParam<unsigned int>("ny");

  if (dim == 3)
    nz = getParam<unsigned int>("nz");

  // We should compute a balanced factorization so
  // that we can assign proper processors to each direction.
  // I just want to make grid partitioner smarter.
  if ((nx * ny * nz) != mesh.n_partitions())
  {
    // Anybody knows we are living in a 3D space.
    int dims[] = {0, 0, 0};
    MPI_Dims_create(mesh.n_partitions(), dim, dims);

    nx = dims[0];
    if (dim >= 2)
      ny = dims[1];
    if (dim == 3)
      nz = dims[2];
  }

  // hx: grid interval along x direction
  // hy: grid interval along y direction
  // hz: grid interval along z direction
  // Lx: domain length along x direction
  // Ly: domain length along y direction
  // Lz: domain length along z direction
  Real hx = 1., hy = 1., hz = 1., Lx = 1., Ly = 1., Lz = 1.;
  Lx = max(0) - min(0);
  hx = Lx / nx;
  if (dim >= 2)
  {
    Ly = max(1) - min(1);
    hy = Ly / ny;
  }

  if (dim == 3)
  {
    Lz = max(2) - min(2);
    hz = Lz / nz;
  }

  // Processor coordinates along x, y, z directions
  unsigned int k = 0, j = 0, i = 0;
  // Coordinates for current element centroid
  Real coordx = 0, coordy = 0, coordz = 0;

  // Loop over all of the elements in the given mesh
  for (auto & elem_ptr : mesh.active_element_ptr_range())
  {
    // Find the element it lands in in the GeneratedMesh
    auto centroid = elem_ptr->vertex_average();

    coordx = centroid(0);
    mooseAssert(coordx >= min(0) && coordy <= max(0),
                "element is outside of bounding box along x direction");
    if (dim >= 2)
    {
      coordy = centroid(1);
      mooseAssert(coordy >= min(1) && coordy <= max(1),
                  "element is outside of bounding box along y direction");
    }
    if (dim == 3)
    {
      coordz = centroid(2);
      mooseAssert(coordz >= min(2) && coordz <= max(2),
                  "element is outside of bounding box along z direction");
    }

    // Compute processor coordinates
    j = k = 0;
    i = (coordx - min(0)) / hx;
    if (dim >= 2)
      j = (coordy - min(1)) / hy;
    if (dim == 3)
      k = (coordz - min(2)) / hz;

    mooseAssert(i < nx, "Index caculation is wrong along x direction");
    mooseAssert(j < ny, "Index caculation is wrong along y direction");
    mooseAssert(k < nz, "Index caculation is wrong along z direction");
    // Assign processor ID to current element
    elem_ptr->processor_id() = k * nx * ny + j * nx + i;
  }
}
