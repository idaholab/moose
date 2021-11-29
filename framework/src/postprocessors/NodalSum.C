//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalSum.h"
#include "MooseMesh.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", NodalSum);

InputParameters
NodalSum::validParams()
{
  InputParameters params = NodalVariablePostprocessor::validParams();
  params.set<bool>("unique_node_execute") = true;

  params.addClassDescription("Computes the sum of all of the nodal values of the specified "
                             "variable. Note: This object sets the default \"unique_node_execute\" "
                             "flag to true to avoid double counting nodes between shared blocks.");
  return params;
}

NodalSum::NodalSum(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _sum(0)
{
}

void
NodalSum::initialize()
{
  _sum = 0;
}

void
NodalSum::execute()
{
  _sum += _u[_qp];
}

Real
NodalSum::getValue()
{
  return _sum;
}

void
NodalSum::finalize()
{
  gatherSum(_sum);
}

void
NodalSum::threadJoin(const UserObject & y)
{
  const NodalSum & pps = static_cast<const NodalSum &>(y);
  _sum += pps._sum;
}
