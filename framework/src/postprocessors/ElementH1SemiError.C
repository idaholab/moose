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

#include "ElementH1SemiError.h"
#include "Function.h"

template<>
InputParameters validParams<ElementH1SemiError>()
{
  InputParameters params = validParams<ElementIntegral>();
  params.addRequiredParam<std::string>("function", "The analytic solution to compare against");
  return params;
}

ElementH1SemiError::ElementH1SemiError(const std::string & name,
                             MooseSystem & moose_system,
                             InputParameters parameters):
  ElementIntegral(name, moose_system, parameters),
  _func(getFunction("function"))
{
}

Real
ElementH1SemiError::getValue()
{
  return std::sqrt(ElementIntegral::getValue());
}

Real
ElementH1SemiError::computeQpIntegral()
{
  RealGradient diff = _grad_u[_qp]-_func.gradient(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
  return diff*diff;
}
