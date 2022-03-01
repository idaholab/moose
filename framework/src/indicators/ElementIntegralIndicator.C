//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralIndicator.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

InputParameters
ElementIntegralIndicator::validParams()
{
  InputParameters params = ElementIndicator::validParams();
  return params;
}

ElementIntegralIndicator::ElementIntegralIndicator(const InputParameters & parameters)
  : ElementIndicator(parameters)
{
}

void
ElementIntegralIndicator::computeIndicator()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();

  //  sum = std::sqrt(sum);

  _field_var.setNodalValue(sum);
}

Real
ElementIntegralIndicator::computeQpIntegral()
{
  return _u[_qp];
}
