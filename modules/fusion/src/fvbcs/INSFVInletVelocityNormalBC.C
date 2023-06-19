//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVInletVelocityNormalBC.h"
#include "INSFVVelocityVariable.h"

registerMooseObject("NavierStokesApp", INSFVInletVelocityNormalBC);

InputParameters
INSFVInletVelocityNormalBC::validParams()
{
  InputParameters params = FVFunctionalNormalDirichletBC::validParams();
  params += INSFVFlowBC::validParams();
  return params;
}

INSFVInletVelocityNormalBC::INSFVInletVelocityNormalBC(const InputParameters & params)
  : FVFunctionalNormalDirichletBC(params), INSFVFlowBC(params)
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to INSFVInletVelocityBC must be of type INSFVVelocityVariable");
}
