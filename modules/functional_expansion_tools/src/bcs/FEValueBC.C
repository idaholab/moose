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

#include "FEValueBC.h"

// Module includes
#include "FunctionSeries.h"

template <>
InputParameters
validParams<FEValueBC>()
{
  InputParameters params = validParams<FunctionDirichletBC>();

  params.addClassDescription(
      "Imposes a fixed value boundary condition, evaluated using a FunctionSeries"
      " instance.");

  return params;
}

FEValueBC::FEValueBC(const InputParameters & parameters) : FunctionDirichletBC(parameters)
{
  FunctionSeries & fe_basis = FunctionSeries::checkAndConvertFunction(_func, name());

  fe_basis.useCache(true);
}
