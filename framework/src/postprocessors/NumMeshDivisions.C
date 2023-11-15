//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumMeshDivisions.h"
#include "MeshDivision.h"
#include "FEProblemBase.h"

registerMooseObject("MooseApp", NumMeshDivisions);

InputParameters
NumMeshDivisions::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<MeshDivisionName>(
      "mesh_division", "MeshDivision object to count the number of divisions/regions from");

  params.addClassDescription("Return the number of divisions/regions from a MeshDivision object.");
  return params;
}

NumMeshDivisions::NumMeshDivisions(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh_division(_fe_problem.getMeshDivision(getParam<MeshDivisionName>("mesh_division")))
{
}

Real
NumMeshDivisions::getValue() const
{
  return _mesh_division.getNumDivisions();
}
