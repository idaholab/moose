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

#include "ElementIntegralPostprocessor.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ElementIntegralPostprocessor>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  return params;
}

ElementIntegralPostprocessor::ElementIntegralPostprocessor(const InputParameters & parameters) :
    ElementPostprocessor(parameters),
    _qp(0),
    _integral_value(0)
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);// ZZY
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in ElementVectorL2Error");
  _xfem = fe_problem->get_xfem();
}

void
ElementIntegralPostprocessor::initialize()
{
  _integral_value = 0;
}

void
ElementIntegralPostprocessor::execute()
{
  _integral_value += computeIntegral();
}

Real
ElementIntegralPostprocessor::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

void
ElementIntegralPostprocessor::threadJoin(const UserObject & y)
{
  const ElementIntegralPostprocessor & pps = static_cast<const ElementIntegralPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
ElementIntegralPostprocessor::computeIntegral()
{
  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    Real inside_flag = _xfem->flag_qp_inside(_current_elem, _q_point[_qp]); // ZZY hack
//    Real radius = _q_point[_qp].size(); // plate with a circular hole
//    if ((radius < 0.4733)) inside_flag = 0.0;

//    Real a = 0.63, b = 0.27; // plate with an elliptic hole
//    Real val = (_q_point[_qp](0)/b)*(_q_point[_qp](0)/b) + (_q_point[_qp](1)/a)*(_q_point[_qp](1)/a);
//    if ((val < 1.0)) inside_flag = 0.0;
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral()*inside_flag;
  }
  return sum;
}

