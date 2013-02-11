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

#include "NodalL2Norm.h"

#include <algorithm>
#include <limits>

template<>
InputParameters validParams<NodalL2Norm>()
{
  InputParameters params = validParams<NodalVariablePostprocessor>();
  return params;
}

NodalL2Norm::NodalL2Norm(const std::string & name, InputParameters parameters) :
  NodalVariablePostprocessor(name, parameters),
  _sum_of_squares(0.0)
{}

void
NodalL2Norm::initialize()
{
  _sum_of_squares = 0.0;
}

void
NodalL2Norm::execute()
{
  Real val = _u[_qp];
  _sum_of_squares += val*val;
}

Real
NodalL2Norm::getValue()
{
  gatherSum(_sum_of_squares);
  return std::sqrt(_sum_of_squares);
}

void
NodalL2Norm::threadJoin(const UserObject & y)
{
  const NodalL2Norm & pps = static_cast<const NodalL2Norm &>(y);
  _sum_of_squares += pps._sum_of_squares;
}
