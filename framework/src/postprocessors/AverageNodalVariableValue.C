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

registerMooseObject("MooseApp", AverageNodalVariableValue);

InputParameters
AverageNodalVariableValue::validParams()
{
  InputParameters params = NodalVariablePostprocessor::validParams();

  params.addClassDescription("Computes the average value of a field by sampling all nodal "
                             "solutions on the domain or within a subdomain");
  return params;
}

AverageNodalVariableValue::AverageNodalVariableValue(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _sum(0), _n(0)
{
}

// doco-init-start
void
AverageNodalVariableValue::initialize()
{
  _sum = 0;
  _n = 0;
}
// doco-init-end

// doco-execute-get-start
void
AverageNodalVariableValue::execute()
{
  _sum += _u[_qp];
  _n++;
}

Real
AverageNodalVariableValue::getValue()
{
  return _sum / _n;
}
// doco-execute-get-end

// doco-final-start
void
AverageNodalVariableValue::finalize()
{
  gatherSum(_sum);
  gatherSum(_n);
}
// doco-final-end

// doco-thread-start
void
AverageNodalVariableValue::threadJoin(const UserObject & y)
{
  const AverageNodalVariableValue & pps = static_cast<const AverageNodalVariableValue &>(y);
  _sum += pps._sum;
  _n += pps._n;
}
// doco-thread-end
