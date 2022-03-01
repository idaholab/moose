//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "VerifyNodalUniqueID.h"
#include "SubProblem.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", VerifyNodalUniqueID);

InputParameters
VerifyNodalUniqueID::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params.addClassDescription("Verifies that all node ids are unique.");
  return params;
}

VerifyNodalUniqueID::VerifyNodalUniqueID(const InputParameters & parameters)
  : NodalUserObject(parameters)
{
}

// This object can't test every possible scenario.  For instance, it can't detect recycled ids
// It's only designed to make sure that all ids are unique in the mesh at any specified execution
void
VerifyNodalUniqueID::initialize()
{
  _all_ids.clear();
  _all_ids.reserve(_subproblem.mesh().getMesh().n_local_nodes());
}

void
VerifyNodalUniqueID::execute()
{
#ifdef LIBMESH_ENABLE_UNIQUE_ID
  _all_ids.push_back(_current_node->unique_id());
#else
  _all_ids.push_back(0);
#endif
}

void
VerifyNodalUniqueID::threadJoin(const UserObject & y)
{
  const VerifyNodalUniqueID & uo = static_cast<const VerifyNodalUniqueID &>(y);

  _all_ids.insert(_all_ids.end(), uo._all_ids.begin(), uo._all_ids.end());
}

void
VerifyNodalUniqueID::finalize()
{
  // On Parallel Mesh we have to look at all the ids over all the processors
  if (_subproblem.mesh().isDistributedMesh())
    _communicator.allgather(_all_ids);

  std::sort(_all_ids.begin(), _all_ids.end());
  std::vector<dof_id_type>::iterator it_end = std::unique(_all_ids.begin(), _all_ids.end());
  if (it_end != _all_ids.end())
    mooseError("Duplicate unique_ids found!");
}
