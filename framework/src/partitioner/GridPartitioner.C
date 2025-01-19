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
  InputParameters params = MoosePartitioner::validParams();

  MooseEnum method("manual automatic", "manual");
  params.addParam<MooseEnum>(
      "grid_computation",
      method,
      "Whether to determine the grid manually (using nx, ny and nz) or automatically. When using "
      "the automatic mode, the user can impose a certain value for nx, ny or nz, and the automatic "
      "factorization will adjust the number of processors in the other directions.");

  // Users specify how many processors they need along each direction
  params.addParam<unsigned int>(
      "nx", "Number of processors in the X direction. Defaults to 1 in manual mode");
  params.addParam<unsigned int>(
      "ny", "Number of processors in the Y direction. Defaults to 1 in manual mode");
  params.addParam<unsigned int>(
      "nz", "Number of processors in the Z direction. Defaults to 1 in manual mode");

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
  return _app.getFactory().clone(*this);
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

  // Gather user parameters
  nx = isParamValid("nx") ? getParam<unsigned int>("nx") : nx;
  if (dim >= 2)
    ny = isParamValid("ny") ? getParam<unsigned int>("ny") : ny;
  if (dim == 3)
    nz = isParamValid("nz") ? getParam<unsigned int>("nz") : nz;

  // simple info message unused parameters as this can be normal: we could be partitioning
  // a 2D mesh before extruding it to 3D. The nz parameter is needed for the 3D mesh
  if (dim < 2 && isParamValid("ny") && getParam<unsigned int>("ny") > 1)
    paramInfo("ny", "Parameter ignored as mesh is currently of dimension less than 2.");
  if (dim < 3 && isParamValid("nz") && getParam<unsigned int>("nz") > 1)
    paramInfo("nz", "Parameter ignored as mesh is currently of dimension less than 3.");

  // User parameters, which should match the number of partitions needed
  if (getParam<MooseEnum>("grid_computation") == "manual")
  {
    //  Need to make sure the number of grid cells matches the number of procs to partition for
    if ((nx * ny * nz) != mesh.n_partitions())
      mooseError("Number of grid cells (" + std::to_string(nx * ny * nz) +
                 ") does not match the number of MPI processes (" +
                 std::to_string(mesh.n_partitions()) + ")");
  }

  else if (getParam<MooseEnum>("grid_computation") == "automatic")
  {
    // remove over-constraint and tell user
    if (nx * ny * nz > mesh.n_partitions())
    {
      nx = 0;
      ny = 0;
      nz = 0;
      paramWarning("grid_computation",
                   "User specified (nx,ny,nz) grid exceeded number of partitions, these parameters "
                   "will be ignored.");
    }
    // 0 means no restriction on which number to choose
    int dims[] = {isParamValid("nx") ? int(nx) : 0,
                  isParamValid("ny") ? int(ny) : 0,
                  isParamValid("nz") ? int(nz) : 0};

    if ((dims[0] > 0 && dim == 1 && dims[0] != int(mesh.n_partitions())) ||
        (dims[0] > 0 && dims[1] > 0 && dim == 2 && dims[0] * dims[1] != int(mesh.n_partitions())) ||
        (dims[0] > 0 && dims[1] > 0 && dims[2] > 0 && dim == 3 &&
         dims[0] * dims[1] * dims[2] != int(mesh.n_partitions())))
    {
      dims[0] = 0;
      dims[1] = 0;
      dims[2] = 0;
      paramWarning("grid_computation",
                   "User specified grid for the current dimension of the mesh (" +
                       std::to_string(dim) + ") does not fit the number of partitions (" +
                       std::to_string(mesh.n_partitions()) +
                       ") and constrain the grid partitioner in every direction, these parameters "
                       "will be ignored.");
    }

    // This will error if the factorization is not possible
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

    mooseAssert(i < nx, "Index calculation is wrong along x direction");
    mooseAssert(j < ny || ny == 0, "Index calculation is wrong along y direction");
    mooseAssert(k < nz || nz == 0, "Index calculation is wrong along z direction");
    // Assign processor ID to current element
    elem_ptr->processor_id() = k * nx * ny + j * nx + i;
  }
}
