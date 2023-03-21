//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVAveragePressureValueBC.h"
#include "INSFVPressureVariable.h"

registerMooseObject("NavierStokesApp", INSFVAveragePressureValueBC);

InputParameters
INSFVAveragePressureValueBC::validParams()
{
  InputParameters params = FVBoundaryIntegralValueConstraint::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();
  return params;
}

INSFVAveragePressureValueBC::INSFVAveragePressureValueBC(const InputParameters & params)
  : FVBoundaryIntegralValueConstraint(params), INSFVFullyDevelopedFlowBC(params)
{
  if (!dynamic_cast<INSFVPressureVariable *>(&_var))
    paramError("variable",
               "The variable argument to INSFVAveragePressureValueBC must be of type "
               "INSFVPressureVariable");
}
