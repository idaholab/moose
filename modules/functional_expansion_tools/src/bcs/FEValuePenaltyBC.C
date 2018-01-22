// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEValuePenaltyBC.h"

// Module includes
#include "FunctionSeries.h"

template <>
InputParameters
validParams<FEValuePenaltyBC>()
{
  InputParameters params = validParams<FunctionPenaltyDirichletBC>();

  params.addClassDescription(
      "Sets a value boundary condition, evaluated using a FunctionSeries instance. This does not "
      "fix the value, but rather 'strongly encourages'  value agreement by penalizing the "
      "differences through contributions to  the residual.");

  return params;
}

FEValuePenaltyBC::FEValuePenaltyBC(const InputParameters & parameters)
  : FunctionPenaltyDirichletBC(parameters)
{
  /*
   * The _func member was made private in FunctionPenaltyDirichletBC, so manually acquire the
   * function so that we can enable the cache (memoization)
   */
  FunctionSeries & fe_basis =
      FunctionSeries::checkAndConvertFunction(getFunction("function"), name());

  fe_basis.useCache(true);
}
