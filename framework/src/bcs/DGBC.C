#include "MooseSystem.h"
#include "DGBC.h"
#include "ElementData.h"
#include "Function.h"

#include "numeric_vector.h"

template<>
InputParameters validParams<DGBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.set<bool>("_integrated") = true;
  params.addRequiredParam<std::string>("function", "The forcing function.");
  params.addParam<Real>("pen", 0.0, "The penalty");
  return params;
}

DGBC::DGBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
   _func(getFunction("function")),
   _pen(parameters.get<Real>("pen"))
{}

Real
DGBC::computeQpResidual()
{
//  Real fn = _func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
/*
  Real fn = 0;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);

  Real theta = atan2(y, x);

  if (theta < 0) theta += 2.0 * libMesh::pi;
  fn = pow(x*x + y*y, 1./3.) * sin(2./3. * theta);

//  return (_u[_qp]-fn) * _test[_i][_qp] - _grad_test[_i][_qp] * _normals[_qp] * (_u[_qp]-fn);
//  return (_pen * (_u[_qp]-fn) * _test[_i][_qp]) - (_grad_test[_i][_qp] * _normals[_qp] * fn);
  return  -(_grad_u[_qp] * _normals[_qp] * _test[_i][_qp]) + (_u[_qp] - fn) * _grad_test[_i][_qp] * _normals[_qp] + _pen * (_u[_qp] - fn) * _test[_i][_qp];
//  return  -(_grad_test[_i][_qp] * _normals[_qp] * (fn));
  */

  const unsigned int elem_b_order = static_cast<unsigned int> (_fe->get_order());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./pow(elem_b_order, 2.);

  Real fn = _func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
  return  -(_grad_u[_qp] * _normals[_qp] * _test[_i][_qp]) + (_u[_qp] - fn) * _grad_test[_i][_qp] * _normals[_qp] + _pen/h_elem * (_u[_qp] - fn) * _test[_i][_qp];
}
