//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXValueBC.h"
#include "FunctionSeries.h"

registerMooseObject("FunctionalExpansionToolsApp", FXValueBC);

InputParameters
FXValueBC::validParams()
{
  InputParameters params = FunctionDirichletBC::validParams();

  params.addClassDescription(
      "Imposes a fixed value boundary condition, evaluated using a FunctionSeries instance.");

  return params;
}

FXValueBC::FXValueBC(const InputParameters & parameters) : FunctionDirichletBC(parameters)
{
  FunctionSeries & fe_basis =
      FunctionSeries::checkAndConvertFunction(_func, getParam<std::string>("_moose_base"), name());

  fe_basis.useCache(true);
}
