/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
