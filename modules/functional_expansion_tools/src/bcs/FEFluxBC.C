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

#include "FEFluxBC.h"

// Module includes
#include "FunctionSeries.h"

template <>
InputParameters
validParams<FEFluxBC>()
{
  InputParameters params = validParams<FunctionNeumannBC>();

  params.addClassDescription(
      "Sets a flux boundary condition, evaluated using a FunctionSeries  instance. This does not "
      "fix the flux, but rather 'strongly encourages' flux agreement by penalizing the differences "
      "through contributions to the residual.");

  return params;
}

FEFluxBC::FEFluxBC(const InputParameters & parameters) : FunctionNeumannBC(parameters)
{
  FunctionSeries & fe_basis = FunctionSeries::checkAndConvertFunction(_func, name());

  fe_basis.useCache(true);
}
