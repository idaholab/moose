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
    _partitioner = new MetisPartitioner;
    break;
  case -1: // parmetis
    _partitioner = new ParmetisPartitioner;
    break;

  case 0: // linear
    _partitioner = new LinearPartitioner;
    break;
  case 1: // centroid
  {
    if (!isParamValid("centroid_partitioner_direction"))
      mooseError("If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

    MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

    if (direction == "x")
      _partitioner = new CentroidPartitioner(CentroidPartitioner::X);
    else if (direction == "y")
      _partitioner = new CentroidPartitioner(CentroidPartitioner::Y);
    else if (direction == "z")
      _partitioner = new CentroidPartitioner(CentroidPartitioner::Z);
    else if (direction == "radial")
      _partitioner = new CentroidPartitioner(CentroidPartitioner::RADIAL);
    break;
  }
  case 2: // hilbert_sfc
    _partitioner = new HilbertSFCPartitioner;
    break;
  case 3: // morton_sfc
    _partitioner = new MortonSFCPartitioner;
    break;
  }
}

LibmeshPartitioner::~LibmeshPartitioner()
{
}

Partitioner *
LibmeshPartitioner::getPartitioner()
{
  return _partitioner;
}
