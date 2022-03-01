//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakMeshByBlockGeneratorBase.h"
#include "InputParameters.h"

InputParameters
BreakMeshByBlockGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addParam<std::string>(
      "interface_name",
      "interface",
      "the name of the new interface. Cannot be used whit `split_interface=true`");
  params.addParam<bool>("split_interface",
                        false,
                        "If true, it creates a "
                        "different interface for each block pair.");
  params.addClassDescription("This is the base class used to split a monolithic"
                             "mesh by blocks pairs");

  return params;
}

BreakMeshByBlockGeneratorBase::BreakMeshByBlockGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _interface_name(getParam<std::string>("interface_name")),
    _split_interface(getParam<bool>("split_interface"))
{
  // check input consistency
  if (getParam<bool>("split_interface") && _pars.isParamSetByUser("interface_name"))
  {
    mooseError("if split_interface == true, the new interface_name"
               " cannot be specified by the user. It will be automatically assigned");
  }
}

boundary_id_type
BreakMeshByBlockGeneratorBase::findFreeBoundaryId(MeshBase & mesh)
{
  const std::set<boundary_id_type> & currentBoundaryIds =
      mesh.get_boundary_info().get_boundary_ids();
  bool freeBoundaryNotFound = true;
  boundary_id_type freeId;
  for (freeId = 0; freeId < std::numeric_limits<boundary_id_type>::max(); freeId++)
  {
    if (currentBoundaryIds.count(freeId) == 0)
    {
      // bid is not in the set, boundaryID is free
      freeBoundaryNotFound = false;
      break;
    }
  }

  if (freeBoundaryNotFound)
    mooseError("Too many boundaries. Maximum limit exceeded!");

  return freeId;
}

std::string
BreakMeshByBlockGeneratorBase::generateBoundaryName(MeshBase & mesh,
                                                    const subdomain_id_type & primaryBlockID,
                                                    const subdomain_id_type & secondaryBlockID)
{
  std::string primary_block_name = mesh.subdomain_name(primaryBlockID);
  std::string secondary_block_name = mesh.subdomain_name(secondaryBlockID);
  if (primary_block_name.empty())
    primary_block_name = "Block" + std::to_string(primaryBlockID);
  if (secondary_block_name.empty())
    secondary_block_name = "Block" + std::to_string(secondaryBlockID);

  return primary_block_name + "_" + secondary_block_name;
}

void
BreakMeshByBlockGeneratorBase::mapBoundaryIdAndBoundaryName(boundary_id_type & boundaryID,
                                                            const std::string & boundaryName)
{
  _bName_bID_set.insert(std::pair<std::string, int>(boundaryName, boundaryID));
}

void
BreakMeshByBlockGeneratorBase::findBoundaryNameAndInd(MeshBase & mesh,
                                                      const subdomain_id_type & primaryBlockID,
                                                      const subdomain_id_type & secondaryBlockID,
                                                      std::string & boundaryName,
                                                      boundary_id_type & boundaryID,
                                                      BoundaryInfo & boundary_info)
{
  // TODO need to be updated if distributed mesh is implemented
  // comments are left to ease implementation

  // mpi barrier
  // first check which boundary name will be created
  boundaryName = generateBoundaryName(mesh, primaryBlockID, secondaryBlockID);

  // check if the boundary name already exist
  bool checkBoundaryAlreadyExist = false;
  for (auto b : _bName_bID_set)
  {
    if (b.first.compare(boundaryName) == 0)
    {
      boundaryID = b.second;
      checkBoundaryAlreadyExist = true;
    }
  }

  if (checkBoundaryAlreadyExist)
  {
    // mpi barrier end
    return;
  }
  else
  {
    boundaryID = findFreeBoundaryId(mesh);
    mapBoundaryIdAndBoundaryName(boundaryID, boundaryName);

    boundary_info.sideset_name(boundaryID) = boundaryName;

    return;
  }
}
