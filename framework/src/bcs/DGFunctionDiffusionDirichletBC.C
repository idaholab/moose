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

#include "MooseSystem.h"
#include "DGFunctionDiffusionDirichletBC.h"
#include "ElementData.h"
#include "Function.h"

#include "numeric_vector.h"

template<>
InputParameters validParams<DGFunctionDiffusionDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.set<bool>("_integrated") = true;
  params.addRequiredParam<std::string>("function", "The forcing function.");
  params.addRequiredParam<Real>("epsilon", "Epsilon");
  params.addRequiredParam<Real>("sigma", "Sigma");

  return params;
}

DGFunctionDiffusionDirichletBC::DGFunctionDiffusionDirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
   _func(getFunction("function")),
   _epsilon(parameters.get<Real>("epsilon")),
   _sigma(parameters.get<Real>("sigma"))
{}

Real
DGFunctionDiffusionDirichletBC::computeQpResidual()
{
  const unsigned int elem_b_order = static_cast<unsigned int> (_fe->get_order());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./pow(elem_b_order, 2.);

  Real fn = _func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
  Real r = 0;
  r -= (_grad_u[_qp] * _normals[_qp] * _test[_i][_qp]);
  r += _epsilon * (fn - _u[_qp]) * _grad_test[_i][_qp] * _normals[_qp];
  r += _sigma/h_elem * (_u[_qp] - fn) * _test[_i][_qp];

  return r;
}

Real
DGFunctionDiffusionDirichletBC::computeQpJacobian()
{
  const unsigned int elem_b_order = static_cast<unsigned int> (_fe->get_order());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./pow(elem_b_order, 2.);

  Real fn = _func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
  Real r = 0;
  r -= (_grad_test[_j][_qp] * _normals[_qp] * _test[_i][_qp]);
  r -= _epsilon * _test[_j][_qp] * _grad_test[_i][_qp] * _normals[_qp];
  r += _sigma/h_elem * _test[_j][_qp] * _test[_i][_qp];
  return r;
}
