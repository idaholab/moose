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
#include "SteadyWithNull.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"

template <>
InputParameters
validParams<SteadyWithNull>()
{
  InputParameters params = validParams<Steady>();
  return params;
}

SteadyWithNull::SteadyWithNull(const InputParameters & parameters) : Steady(parameters) {}

void
SteadyWithNull::init()
{
  Steady::init();
  NumericVector<Number> * to_vector1 = &_problem.getNonlinearSystemBase().getVector("NullSpace_0");
  const NumericVector<Number> * from_vector = _problem.getAuxiliarySystem().currentSolution();
  *to_vector1 = *from_vector;
  if (_problem.subspaceDim("TransposeNullSpace") > 0)
  {
    NumericVector<Number> * to_vector2 =
        &_problem.getNonlinearSystemBase().getVector("TransposeNullSpace_0");
    *to_vector2 = *from_vector;
  }
  _problem.getNonlinearSystemBase().update();
}
