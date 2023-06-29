//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVariableStatistics.h"

registerMooseObject("MooseApp", NodalVariableStatistics);

InputParameters
NodalVariableStatistics::validParams()
{
  InputParameters params = NodalStatistics::validParams();

  params.addRequiredCoupledVar("coupled_var", "Coupled variable whose value is used.");

  params.addClassDescription("Nodal reporter to get statistics for a coupled variable. This can "
                             "be transfered to other apps.");
  return params;
}

NodalVariableStatistics::NodalVariableStatistics(const InputParameters & parameters)
  : NodalStatistics(parameters), _v(coupledValue("coupled_var"))
{
}

Real
NodalVariableStatistics::computeValue()
{
  return _v[_qp];
}
