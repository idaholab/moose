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

// MOOSE includes
#include "NumAdaptivityCycles.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<NumAdaptivityCycles>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

NumAdaptivityCycles::NumAdaptivityCycles(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
}

Real
NumAdaptivityCycles::getValue()
{
  return _fe_problem.getNumCyclesCompleted();
}
