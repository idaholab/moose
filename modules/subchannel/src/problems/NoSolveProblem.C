//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NoSolveProblem.h"

registerMooseObject("SubChannelApp", NoSolveProblem);

InputParameters
NoSolveProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addClassDescription("Dummy problem class that doesn't solve anything");
  return params;
}

NoSolveProblem::NoSolveProblem(const InputParameters & params) : ExternalProblem(params) {}

void
NoSolveProblem::externalSolve()
{
}

void NoSolveProblem::syncSolutions(Direction /*direction*/) {}

bool
NoSolveProblem::solverSystemConverged(const unsigned int)
{
  return true;
}
