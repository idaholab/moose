//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXValuePenaltyBC.h"
#include "FunctionSeries.h"

registerMooseObject("FunctionalExpansionToolsApp", FXValuePenaltyBC);

InputParameters
FXValuePenaltyBC::validParams()
{
  InputParameters params = FunctionPenaltyDirichletBC::validParams();

  params.addClassDescription(
      "Sets a value boundary condition, evaluated using a FunctionSeries instance. This does not "
      "fix the value, but rather 'strongly encourages'  value agreement by penalizing the "
      "differences through contributions to  the residual.");

  return params;
}

FXValuePenaltyBC::FXValuePenaltyBC(const InputParameters & parameters)
  : FunctionPenaltyDirichletBC(parameters)
{
  FunctionSeries & fe_basis =
      FunctionSeries::checkAndConvertFunction(_func, getParam<std::string>("_moose_base"), name());

  fe_basis.useCache(true);
}
