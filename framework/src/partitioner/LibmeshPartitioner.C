//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibmeshPartitioner.h"

#include "MooseMeshUtils.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/centroid_partitioner.h"
#include "libmesh/parmetis_partitioner.h"
#include "libmesh/metis_partitioner.h"
#include "libmesh/hilbert_sfc_partitioner.h"
#include "libmesh/morton_sfc_partitioner.h"
#include "libmesh/subdomain_partitioner.h"

registerMooseObject("MooseApp", LibmeshPartitioner);

InputParameters
LibmeshPartitioner::validParams()
{
  InputParameters params = MoosePartitioner::validParams();
  params.addClassDescription("Mesh partitioning using capabilities defined in libMesh.");
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
    _subdomain_blocks(getParam<std::vector<std::vector<SubdomainName>>>("blocks"))
{
  switch (_partitioner_name)
  {
    case -2: // metis
      _partitioner = std::make_unique<MetisPartitioner>();
      break;
    case -1: // parmetis
      _partitioner = std::make_unique<ParmetisPartitioner>();
      break;

    case 0: // linear
      _partitioner = std::make_unique<LinearPartitioner>();
      break;
    case 1: // centroid
    {
      if (!isParamValid("centroid_partitioner_direction"))
        mooseError(
            "If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

      MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

      if (direction == "x")
        _partitioner = std::make_unique<CentroidPartitioner>(CentroidPartitioner::X);
      else if (direction == "y")
        _partitioner = std::make_unique<CentroidPartitioner>(CentroidPartitioner::Y);
      else if (direction == "z")
        _partitioner = std::make_unique<CentroidPartitioner>(CentroidPartitioner::Z);
      else if (direction == "radial")
        _partitioner = std::make_unique<CentroidPartitioner>(CentroidPartitioner::RADIAL);
      break;
    }
    case 2: // hilbert_sfc
      _partitioner = std::make_unique<HilbertSFCPartitioner>();
      break;
    case 3: // morton_sfc
      _partitioner = std::make_unique<MortonSFCPartitioner>();
      break;
    case 4: // subdomain_partitioner
      _partitioner = std::make_unique<SubdomainPartitioner>();
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
      return std::make_unique<MetisPartitioner>();

    case -1: // parmetis
      return std::make_unique<ParmetisPartitioner>();

    case 0: // linear
      return std::make_unique<LinearPartitioner>();

    case 1: // centroid
    {
      if (!isParamValid("centroid_partitioner_direction"))
        mooseError(
            "If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

      MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

      if (direction == "x")
        return std::make_unique<CentroidPartitioner>(CentroidPartitioner::X);
      else if (direction == "y")
        return std::make_unique<CentroidPartitioner>(CentroidPartitioner::Y);
      else if (direction == "z")
        return std::make_unique<CentroidPartitioner>(CentroidPartitioner::Z);
      else if (direction == "radial")
        return std::make_unique<CentroidPartitioner>(CentroidPartitioner::RADIAL);
      break;
    }
    case 2: // hilbert_sfc
      return std::make_unique<HilbertSFCPartitioner>();

    case 3: // morton_sfc
      return std::make_unique<MortonSFCPartitioner>();

    case 4: // subdomain_partitioner
      return std::make_unique<LibmeshPartitioner>(parameters());
  }
  // this cannot happen but I need to trick the compiler into
  // believing me
  mooseError("Error in LibmeshPartitioner: Supplied partitioner option causes error in clone()");
  return std::make_unique<MetisPartitioner>();
}

void
LibmeshPartitioner::prepareBlocksForSubdomainPartitioner(
    const MeshBase & mesh, SubdomainPartitioner & subdomain_partitioner)
{
  // For making sure all of the blocks exist
  std::set<subdomain_id_type> mesh_subdomain_ids;
  mesh.subdomain_ids(mesh_subdomain_ids);

  // Clear chunks before filling
  subdomain_partitioner.chunks.clear();

  // Insert each chunk
  for (const auto & group : _subdomain_blocks)
  {
    const auto subdomain_ids = MooseMeshUtils::getSubdomainIDs(mesh, group);
    for (const auto id : subdomain_ids)
      if (!mesh_subdomain_ids.count(id))
        paramError("blocks", "The block ", id, " was not found on the mesh");

    std::set<subdomain_id_type> subdomain_ids_set(subdomain_ids.begin(), subdomain_ids.end());

    subdomain_partitioner.chunks.push_back(subdomain_ids_set);
  }
}

void
LibmeshPartitioner::partition(MeshBase & mesh, const unsigned int n)
{
  if (_partitioner_name == "subdomain_partitioner")
  {
    mooseAssert(_partitioner.get(), "Partitioner is a NULL object");
    prepareBlocksForSubdomainPartitioner(mesh,
                                         static_cast<SubdomainPartitioner &>(*_partitioner.get()));
  }

  _partitioner->partition(mesh, n);
}

void
LibmeshPartitioner::partition(MeshBase & mesh)
{
  if (_partitioner_name == "subdomain_partitioner")
  {
    mooseAssert(_partitioner.get(), "Partitioner is a NULL object");
    prepareBlocksForSubdomainPartitioner(mesh,
                                         static_cast<SubdomainPartitioner &>(*_partitioner.get()));
  }

  _partitioner->partition(mesh);
}

void
LibmeshPartitioner::_do_partition(MeshBase & /*mesh*/, const unsigned int /*n*/)
{
}
