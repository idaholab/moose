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
  InputParameters params = FVADFunctorDirichletBC::validParams();
  params += INSFVFlowBC::validParams();
  params.addParam<FunctionName>("function", "Function (functor) describing the inlet velocity");
  params.deprecateParam("function", "functor", "01/01/2024");
  return params;
}

INSFVInletVelocityBC::INSFVInletVelocityBC(const InputParameters & params)
  : FVADFunctorDirichletBC(params), INSFVFlowBC(params)
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to INSFVInletVelocityBC must be of type INSFVVelocityVariable");
}
