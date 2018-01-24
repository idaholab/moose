//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseMesh.h"

#include "LibmeshPartitioner.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/centroid_partitioner.h"
#include "libmesh/parmetis_partitioner.h"
#include "libmesh/metis_partitioner.h"
#include "libmesh/hilbert_sfc_partitioner.h"
#include "libmesh/morton_sfc_partitioner.h"
#include "libmesh/subdomain_partitioner.h"

template <>
InputParameters
validParams<LibmeshPartitioner>()
{
  InputParameters params = validParams<MoosePartitioner>();
  MooseEnum partitioning(
      "metis=-2 parmetis=-1 linear=0 centroid hilbert_sfc morton_sfc subdomain_partitioner");
  params.addRequiredParam<MooseEnum>(
      "partitioner",
      partitioning,
      "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation.");
  MooseEnum direction("x y z radial");
  params.addParam<MooseEnum>("centroid_partitioner_direction",
                             direction,
                             "Specifies the sort direction if using the centroid partitioner. "
                             "Available options: x, y, z, radial");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "blocks", "Block is seperated by ;, and partition mesh block by block. ");
  return params;
}

LibmeshPartitioner::LibmeshPartitioner(const InputParameters & params)
  : MoosePartitioner(params),
    _partitioner_name(getParam<MooseEnum>("partitioner")),
    _subdomain_blocks(getParam<std::vector<std::vector<SubdomainName>>>("blocks")),
    _mesh(*getParam<MooseMesh *>("mesh"))
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
        mooseError(
            "If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

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
    case 4: // subdomain_partitioner
      _partitioner = libmesh_make_unique<SubdomainPartitioner>();
      break;
  }
}

LibmeshPartitioner::~LibmeshPartitioner() {}

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
        mooseError(
            "If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

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
    case 4: // subdomain_partitioner
      return libmesh_make_unique<LibmeshPartitioner>(parameters());
      break;
  }
  // this cannot happen but I need to trick the compiler into
  // believing me
  mooseError("Error in LibmeshPartitioner: Supplied partitioner option causes error in clone()");
  return libmesh_make_unique<MetisPartitioner>();
}

void
LibmeshPartitioner::prepare_blocks_for_subdomain_partitioner(
    SubdomainPartitioner & subdomain_partitioner)
{
  auto group_begin = _subdomain_blocks.begin();
  auto group_end = _subdomain_blocks.end();

  subdomain_partitioner.chunks.clear();
  for (auto group = group_begin; group != group_end; ++group)
  {
    std::set<subdomain_id_type> subdomain_ids;
    auto subdomain_ids_vec = _mesh.getSubdomainIDs(*group);
    auto subdomain_begin = subdomain_ids_vec.begin();
    auto subdomain_end = subdomain_ids_vec.end();
    for (auto subdomain_id = subdomain_begin; subdomain_id != subdomain_end; ++subdomain_id)
    {
      subdomain_ids.insert(*subdomain_id);
    }
    subdomain_partitioner.chunks.push_back(subdomain_ids);
  }
}

void
LibmeshPartitioner::partition(MeshBase & mesh, const unsigned int n)
{
  if (_partitioner_name == "subdomain_partitioner")
  {
    mooseAssert(_partitioner.get(), "Paritioner is a NULL object");
    prepare_blocks_for_subdomain_partitioner(
        static_cast<SubdomainPartitioner &>(*_partitioner.get()));
  }

  _partitioner->partition(mesh, n);
}

void
LibmeshPartitioner::partition(MeshBase & mesh)
{
  if (_partitioner_name == "subdomain_partitioner")
  {
    mooseAssert(_partitioner.get(), "Paritioner is a NULL object");
    prepare_blocks_for_subdomain_partitioner(
        static_cast<SubdomainPartitioner &>(*_partitioner.get()));
  }

  _partitioner->partition(mesh);
}

void
LibmeshPartitioner::_do_partition(MeshBase & /*mesh*/, const unsigned int /*n*/)
{
}
