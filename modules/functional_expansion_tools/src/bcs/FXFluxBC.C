//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXFluxBC.h"
#include "FunctionSeries.h"

registerMooseObject("FunctionalExpansionToolsApp", FXFluxBC);

InputParameters
FXFluxBC::validParams()
{
  InputParameters params = FunctionNeumannBC::validParams();

  params.addClassDescription(
      "Sets a flux boundary condition, evaluated using a FunctionSeries instance. This does not "
      "fix the flux, but rather 'strongly encourages' flux agreement by penalizing the differences "
      "through contributions to the residual.");

  return params;
}

FXFluxBC::FXFluxBC(const InputParameters & parameters) : FunctionNeumannBC(parameters)
{
  FunctionSeries & fe_basis =
      FunctionSeries::checkAndConvertFunction(_func, getParam<std::string>("_moose_base"), name());

  fe_basis.useCache(true);
}
