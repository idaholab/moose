//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVOutletPressureBC.h"
#include "INSFVPressureVariable.h"

registerMooseObject("NavierStokesApp", INSFVOutletPressureBC);

InputParameters
INSFVOutletPressureBC::validParams()
{
  InputParameters params = FVFunctionDirichletBC::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();
  return params;
}

INSFVOutletPressureBC::INSFVOutletPressureBC(const InputParameters & params)
  : FVFunctionDirichletBC(params), INSFVFullyDevelopedFlowBC(params)
{
  if (!dynamic_cast<INSFVPressureVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to INSFVOutletPressureBC must be of type INSFVPressureVariable");
}
