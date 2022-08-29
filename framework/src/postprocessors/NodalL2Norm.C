//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalL2Norm.h"

registerMooseObject("MooseApp", NodalL2Norm);

InputParameters
NodalL2Norm::validParams()
{
  InputParameters params = NodalVariablePostprocessor::validParams();
  params.addClassDescription(
      "Computes the nodal L2-norm of the coupled variable, which is defined by summing the square "
      "of its value at every node and taking the square root.");
  params.set<bool>("unique_node_execute") = true;
  return params;
}

NodalL2Norm::NodalL2Norm(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _sum_of_squares(0.0)
{
}

void
NodalL2Norm::initialize()
{
  _sum_of_squares = 0.0;
}

void
NodalL2Norm::execute()
{
  Real val = _u[_qp];
  _sum_of_squares += val * val;
}

Real
NodalL2Norm::getValue()
{
  return std::sqrt(_sum_of_squares);
}

void
NodalL2Norm::threadJoin(const UserObject & y)
{
  const NodalL2Norm & pps = static_cast<const NodalL2Norm &>(y);
  _sum_of_squares += pps._sum_of_squares;
}

void
NodalL2Norm::finalize()
{
  gatherSum(_sum_of_squares);
}
