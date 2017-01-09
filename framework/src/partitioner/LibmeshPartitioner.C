/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "LibmeshPartitioner.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/centroid_partitioner.h"
#include "libmesh/parmetis_partitioner.h"
#include "libmesh/metis_partitioner.h"
#include "libmesh/hilbert_sfc_partitioner.h"
#include "libmesh/morton_sfc_partitioner.h"

template<>
InputParameters validParams<LibmeshPartitioner>()
{
  InputParameters params = validParams<MoosePartitioner>();
  MooseEnum partitioning("metis=-2 parmetis=-1 linear=0 centroid hilbert_sfc morton_sfc");
  params.addRequiredParam<MooseEnum>("partitioner", partitioning, "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation.");
  MooseEnum direction("x y z radial");
  params.addParam<MooseEnum>("centroid_partitioner_direction", direction, "Specifies the sort direction if using the centroid partitioner. Available options: x, y, z, radial");
  return params;
}

LibmeshPartitioner::LibmeshPartitioner(const InputParameters & params) :
    MoosePartitioner(params),
    _partitioner_name(getParam<MooseEnum>("partitioner"))
{
  switch (_partitioner_name)
  {
  case -2: // metis
    _partitioner = libmesh_make_unique<MetisPartitioner>();
    break;
  case -1: // parmetis
    _partitioner = libmesh_make_unique<ParmetisPartitioner>();
    break;

  case 0: // linear
    _partitioner = libmesh_make_unique<LinearPartitioner>();
    break;
  case 1: // centroid
  {
    if (!isParamValid("centroid_partitioner_direction"))
      mooseError("If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

    MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

    if (direction == "x")
      _partitioner = libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::X);
    else if (direction == "y")
      _partitioner = libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::Y);
    else if (direction == "z")
      _partitioner = libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::Z);
    else if (direction == "radial")
      _partitioner = libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::RADIAL);
    break;
  }
  case 2: // hilbert_sfc
    _partitioner = libmesh_make_unique<HilbertSFCPartitioner>();
    break;
  case 3: // morton_sfc
    _partitioner = libmesh_make_unique<MortonSFCPartitioner>();
    break;
  }
}

LibmeshPartitioner::~LibmeshPartitioner()
{
}

std::unique_ptr<Partitioner>
LibmeshPartitioner::clone() const
{
  switch (_partitioner_name)
  {
  case -2: // metis
    return libmesh_make_unique<MetisPartitioner>();
    break;
  case -1: // parmetis
    return libmesh_make_unique<ParmetisPartitioner>();
    break;

  case 0: // linear
    return libmesh_make_unique<LinearPartitioner>();
    break;
  case 1: // centroid
  {
    if (!isParamValid("centroid_partitioner_direction"))
      mooseError("If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

    MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

    if (direction == "x")
      return libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::X);
    else if (direction == "y")
      return libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::Y);
    else if (direction == "z")
      return libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::Z);
    else if (direction == "radial")
      return libmesh_make_unique<CentroidPartitioner>(CentroidPartitioner::RADIAL);
    break;
  }
  case 2: // hilbert_sfc
return libmesh_make_unique<HilbertSFCPartitioner>();
    break;
  case 3: // morton_sfc
return libmesh_make_unique<MortonSFCPartitioner>();
    break;
  }
  // this cannot happen but I need to trick the compiler into
  // believing me
  mooseError("Error in LibmeshPartitioner: Supplied partitioner option causes error in clone()");
return libmesh_make_unique<MetisPartitioner>();
}

void
LibmeshPartitioner::partition(MeshBase &mesh, const unsigned int n)
{
  _partitioner->partition(mesh, n);
}

void
LibmeshPartitioner::partition(MeshBase &mesh)
{
  _partitioner->partition(mesh);
}

void
LibmeshPartitioner::_do_partition(MeshBase & /*mesh*/, const unsigned int /*n*/)
{
}
