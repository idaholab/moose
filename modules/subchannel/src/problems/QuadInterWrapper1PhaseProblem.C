//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadInterWrapper1PhaseProblem.h"

registerMooseObject("SubChannelApp", QuadInterWrapper1PhaseProblem);

InputParameters
QuadInterWrapper1PhaseProblem::validParams()
{
  InputParameters params = InterWrapper1PhaseProblem::validParams();
  params.addClassDescription(
      "Solver class for interwrapper of assemblies in a square-lattice arrangement");
  return params;
}

QuadInterWrapper1PhaseProblem::QuadInterWrapper1PhaseProblem(const InputParameters & params)
  : InterWrapper1PhaseProblem(params)
{
}

double
QuadInterWrapper1PhaseProblem::computeFrictionFactor(Real Re)
{
  Real a, b;
  if (Re < 1)
  {
    return 64.0;
  }
  else if (Re >= 1 and Re < 5000)
  {
    a = 64.0;
    b = -1.0;
  }
  else if (Re >= 5000 and Re < 30000)
  {
    a = 0.316;
    b = -0.25;
  }
  else
  {
    a = 0.184;
    b = -0.20;
  }
  return a * std::pow(Re, b);
}
