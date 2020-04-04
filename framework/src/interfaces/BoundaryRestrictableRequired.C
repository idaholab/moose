//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryRestrictableRequired.h"

#include "InputParameters.h"

InputParameters
BoundaryRestrictableRequired::validParams()
{

  // Create instance of InputParameters
  InputParameters params = emptyInputParameters();

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where this boundary condition applies");

  // A parameter for disabling error message for objects restrictable by boundary and block,
  // if the parameter is valid it was already set so don't do anything
  if (!params.isParamValid("_dual_restrictable"))
    params.addPrivateParam<bool>("_dual_restrictable", false);

  return params;
}

BoundaryRestrictableRequired::BoundaryRestrictableRequired(const MooseObject * moose_object,
                                                           bool nodal)
  : BoundaryRestrictable(moose_object, nodal)
{
}
