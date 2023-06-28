//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementVariableStatistics.h"

registerMooseObject("MooseApp", ElementVariableStatistics);

InputParameters
ElementVariableStatistics::validParams()
{
  InputParameters params = ElementStatistics::validParams();

  params.addRequiredCoupledVar("coupled_var", "Coupled variable whose value is used.");

  params.addClassDescription("Element reporter to get statistics for a coupled variable. This can "
                             "be transfered to other apps.");
  return params;
}

ElementVariableStatistics::ElementVariableStatistics(const InputParameters & parameters)
  : ElementStatistics(parameters), _v(coupledValue("coupled_var"))
{
}

Real
ElementVariableStatistics::computeValue()
{
  Real avg_val = 0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    avg_val += _v[qp] * _JxW[qp] * _coord[qp];
  avg_val /= _current_elem_volume;

  return avg_val;
}
