//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "PressureJumpModel.h"

#include "FaceInfo.h"
#include "MooseMesh.h"

InputParameters
PressureJumpModel::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += NonADFunctorInterface::validParams();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The boundaries on which this pressure jump model applies.");

  // Pressure jump models are called directly by PorousRhieChowMassFlux.
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

PressureJumpModel::PressureJumpModel(const InputParameters & params)
  : GeneralUserObject(params),
    NonADFunctorInterface(this),
    _moose_mesh(UserObject::_subproblem.mesh())
{
  const auto ids = _moose_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("boundary"));
  _boundary_ids.insert(ids.begin(), ids.end());
}

bool
PressureJumpModel::appliesTo(const FaceInfo & fi) const
{
  for (const auto boundary_id : fi.boundaryIDs())
    if (_boundary_ids.count(boundary_id))
      return true;

  return false;
}

bool
PressureJumpModel::elemIsOwner(const FaceInfo & fi) const
{
  if (!fi.neighborPtr())
    return true;

  bool elem_has_boundary = false;
  bool neighbor_has_boundary = false;

  for (const auto boundary_id : _moose_mesh.getBoundaryIDs(fi.elemPtr(), fi.elemSideID()))
    if (_boundary_ids.count(boundary_id))
    {
      elem_has_boundary = true;
      break;
    }

  for (const auto boundary_id : _moose_mesh.getBoundaryIDs(fi.neighborPtr(), fi.neighborSideID()))
    if (_boundary_ids.count(boundary_id))
    {
      neighbor_has_boundary = true;
      break;
    }

  if (elem_has_boundary != neighbor_has_boundary)
    return elem_has_boundary;

  return fi.elem().subdomain_id() <= fi.neighbor().subdomain_id();
}
