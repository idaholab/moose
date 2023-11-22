//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDivision.h"
#include "FEProblemBase.h"

InputParameters
MeshDivision::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription(
      "Base class to divide/partition the mesh into separately indexed regions");
  params.registerBase("MeshDivision");
  return params;
}

MeshDivision::MeshDivision(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    MeshChangedInterface(parameters),
    _fe_problem(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _mesh(_fe_problem->mesh()),
    _mesh_fully_indexed(true)
{
}

MeshDivision::~MeshDivision() {}
