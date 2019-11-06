//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SteadyWithNull.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"

registerMooseObject("MooseTestApp", SteadyWithNull);

InputParameters
SteadyWithNull::validParams()
{
  InputParameters params = Steady::validParams();
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
