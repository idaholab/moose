//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageNodalVariableValue.h"
#include "MooseMesh.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<AverageNodalVariableValue>()
{
  InputParameters params = validParams<NodalVariablePostprocessor>();
  return params;
}

AverageNodalVariableValue::AverageNodalVariableValue(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _avg(0), _n(0)
{
}

void
AverageNodalVariableValue::initialize()
{
  _avg = 0;
  _n = 0;
}

void
AverageNodalVariableValue::execute()
{
  _avg += _u[_qp];
  _n++;
}

Real
AverageNodalVariableValue::getValue()
{
  gatherSum(_avg);
  gatherSum(_n);

  return _avg / _n;
}

void
AverageNodalVariableValue::threadJoin(const UserObject & y)
{
  const AverageNodalVariableValue & pps = static_cast<const AverageNodalVariableValue &>(y);
  _avg += pps._avg;
  _n += pps._n;
}
