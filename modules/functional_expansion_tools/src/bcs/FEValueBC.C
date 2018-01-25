//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Module includes
#include "FEValueBC.h"
#include "FunctionSeries.h"

template <>
InputParameters
validParams<FEValueBC>()
{
  InputParameters params = validParams<FunctionDirichletBC>();

  params.addClassDescription(
      "Imposes a fixed value boundary condition, evaluated using a FunctionSeries instance.");

  return params;
}

FEValueBC::FEValueBC(const InputParameters & parameters) : FunctionDirichletBC(parameters)
{
  FunctionSeries & fe_basis = FunctionSeries::checkAndConvertFunction(_func, name());

  fe_basis.useCache(true);
}
