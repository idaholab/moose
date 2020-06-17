//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakMeshByBlockBase.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<BreakMeshByBlockBase>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addParam<std::string>(
      "interface_name",
      "interface",
      "the name of the new interface. Cannot be used whit `split_interface=true`");
  params.addParam<bool>("split_interface",
                        false,
                        "If true, it create a "
                        "different interface for each block pair.");
  params.addClassDescription("This is the base class used to split a monolithic"
                             "mesh by blocks pairs");
  return params;
}

BreakMeshByBlockBase::BreakMeshByBlockBase(const InputParameters & parameters)
  : MeshModifier(parameters),
    _interface_name(getParam<std::string>("interface_name")),
    _split_interface(getParam<bool>("split_interface"))
{
}

void
BreakMeshByBlockBase::modify()
{
  mooseError("BreakMeshByBlockBase should never be called directly!"
             "Always use one of its derived classes");
}

void
BreakMeshByBlockBase::checkInputParameter()
{
  // check input consistency
  if (getParam<bool>("split_interface") && _pars.isParamSetByUser("interface_name"))
  {
    mooseError("if split_interface == true,  the new interface_name"
               " cannot be specified by the user. It will be autoamtically assigned");
  }
}

BoundaryID
BreakMeshByBlockBase::findFreeBoundaryId()
{
  const std::set<BoundaryID> & currentBoundaryIds = _mesh_ptr->getBoundaryIDs();
  bool freeBoundaryNotFound = true;
  BoundaryID freeId;
  for (freeId = 0; freeId < std::numeric_limits<BoundaryID>::max(); freeId++)
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
BreakMeshByBlockBase::generateBoundaryName(const subdomain_id_type & masterBlockID,
                                           const subdomain_id_type & secondaryBlockID)
{
  std::string master_block_name = _mesh_ptr->getSubdomainName(masterBlockID);
  std::string secondary_block_name = _mesh_ptr->getSubdomainName(secondaryBlockID);
  if (master_block_name.empty())
    master_block_name = "Block" + std::to_string(masterBlockID);
  if (secondary_block_name.empty())
    secondary_block_name = "Block" + std::to_string(secondaryBlockID);

  return master_block_name + "_" + secondary_block_name;
}

void
BreakMeshByBlockBase::mapBoundaryIdAndBoundaryName(BoundaryID & boundaryID,
                                                   const std::string & boundaryName)
{
  _bName_bID_set.insert(std::pair<std::string, int>(boundaryName, boundaryID));
}

void
BreakMeshByBlockBase::findBoundaryNameAndInd(const subdomain_id_type & masterBlockID,
                                             const subdomain_id_type & secondaryBlockID,
                                             std::string & boundaryName,
                                             BoundaryID & boundaryID,
                                             BoundaryInfo & boundary_info)
{
  // TODO need to be updated if distributed mesh is implemented
  // comments are left to ease implementation

  // mpi barrier
  // first check which boundary name will be created
  boundaryName = generateBoundaryName(masterBlockID, secondaryBlockID);

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
    boundaryID = findFreeBoundaryId();
    mapBoundaryIdAndBoundaryName(boundaryID, boundaryName);

    boundary_info.sideset_name(boundaryID) = boundaryName;

    return;
  }
}
