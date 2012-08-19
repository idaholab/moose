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

#include "ElementIntegralIndicator.h"

template<>
InputParameters validParams<ElementIntegralIndicator>()
{
  InputParameters params = validParams<ElementIndicator>();
  return params;
}


ElementIntegralIndicator::ElementIntegralIndicator(const std::string & name, InputParameters parameters) :
    ElementIndicator(name, parameters)
{
}

void
ElementIntegralIndicator::computeIndicator()
{
  Real sum = 0;
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();

//  sum = std::sqrt(sum);

  _field_var.setNodalValue(sum);
}

Real
ElementIntegralIndicator::computeQpIntegral()
{
  return _u[_qp];
}

