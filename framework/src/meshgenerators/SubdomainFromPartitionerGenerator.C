//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainFromPartitionerGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/partitioner.h"

registerMooseObject("MooseApp", SubdomainFromPartitionerGenerator);

InputParameters
SubdomainFromPartitionerGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription(
      "Changes the subdomain ID of elements based on the output of the partitioner");
  params.addParam<std::vector<SubdomainName>>(
      "included_subdomains", "Only change subdomain ID only for elements in those subdomains");
  params.addParam<subdomain_id_type>("offset",
                                     0,
                                     "Offset to apply to the subdomain IDs. Default type is a "
                                     "short integer, do not use a large value!");
  params.addRequiredParam<unsigned int>("num_partitions",
                                        "Number of partitioners to get from the partitioner");
  return params;
}

SubdomainFromPartitionerGenerator::SubdomainFromPartitionerGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _offset(getParam<subdomain_id_type>("offset"))
{
}

std::unique_ptr<MeshBase>
SubdomainFromPartitionerGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  if (getParam<unsigned int>("num_partitions") != n_processors())
    paramWarning("num_partitions",
                 "Partitioner may error with num_partitions != number of MPI processors");

  // Process the block restriction, if any
  std::set<SubdomainID> restricted_ids;
  bool has_restriction = getParam<std::vector<SubdomainName>>("included_subdomains").size();
  if (has_restriction)
  {
    const auto & names = getParam<std::vector<SubdomainName>>("included_subdomains");
    for (const auto & name : names)
    {
      // check that the subdomain exists in the mesh
      if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
        paramError("included_subdomains", "The block '", name, "' was not found in the mesh");

      restricted_ids.insert(MooseMeshUtils::getSubdomainID(name, *mesh));
    }
  }

  // Extract the mesh
  auto block_mesh = buildMeshBaseObject();
  if (has_restriction)
    MooseMeshUtils::convertBlockToMesh(
        *mesh, *block_mesh, getParam<std::vector<SubdomainName>>("included_subdomains"));
  else
    block_mesh = mesh->clone();

  // Get the partitioner
  const auto & partitioner = mesh->partitioner();

  // Partition this mesh
  partitioner->partition(*block_mesh, getParam<unsigned int>("num_partitions"));
  auto pl = block_mesh->sub_point_locator();

  // Loop over the elements
  for (const auto & elem : mesh->element_ptr_range())
  {
    if (has_restriction && restricted_ids.count(elem->subdomain_id()) == 0)
      continue;

    // Get the element from point locator
    const Elem * const block_mesh_elem = (*pl)(elem->vertex_average());

    // Get a new subdomain id from the partition of the block mesh
    elem->subdomain_id() = _offset + block_mesh_elem->processor_id();
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
