//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVInletVelocityBC.h"
#include "INSFVVelocityVariable.h"

registerMooseObject("NavierStokesApp", INSFVInletVelocityBC);

InputParameters
INSFVInletVelocityBC::validParams()
{
  InputParameters params = FVFunctionDirichletBC::validParams();
  params += INSFVFlowBC::validParams();
  return params;
}

INSFVInletVelocityBC::INSFVInletVelocityBC(const InputParameters & params)
  : FVFunctionDirichletBC(params), INSFVFlowBC(params)
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to INSFVInletVelocityBC must be of type INSFVVelocityVariable");
}
