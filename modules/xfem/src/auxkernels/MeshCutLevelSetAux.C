//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCutLevelSetAux.h"
#include "InterfaceMeshCutUserObjectBase.h"

registerMooseObject("XFEMApp", MeshCutLevelSetAux);

InputParameters
MeshCutLevelSetAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates signed distance from interface defined by InterfaceMeshCutUserObject.");
  params.addParam<UserObjectName>(
      "mesh_cut_user_object",
      "Name of InterfaceMeshCutUserObject that gives cut mesh information.");
  return params;
}

MeshCutLevelSetAux::MeshCutLevelSetAux(const InputParameters & parameters) : AuxKernel(parameters)
{
  if (!isNodal())
    mooseError("MeshCutLevelSetAux: Aux variable must be nodal variable.");

  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);

  const UserObject * uo =
      &(fe_problem->getUserObjectBase(getParam<UserObjectName>("mesh_cut_user_object")));

  if (dynamic_cast<const InterfaceMeshCutUserObjectBase *>(uo) == nullptr)
    mooseError("Failed to cast UserObject to InterfaceMeshCutUserObjectBase in MeshCutLevelSetAux");

  _mesh_cut_uo = dynamic_cast<const InterfaceMeshCutUserObjectBase *>(uo);
}

Real
MeshCutLevelSetAux::computeValue()
{
  return _mesh_cut_uo->calculateSignedDistance(*_current_node);
}
