#include "NIPG0.h"

template<>
InputParameters validParams<NIPG0>()
{
  InputParameters params = validParams<DGKernel>();
  params.addRequiredParam<Real>("sigma", "Sigma");
  params.addRequiredParam<Real>("e", "e");
  return params;
}

NIPG0::NIPG0(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :DGKernel(name, moose_system, parameters),
   _e(parameters.get<Real>("e")),
   _s(parameters.get<Real>("sigma"))
{
}

Real
NIPG0::computeQpResidual()
{
  Real r = 0;

  r -= 0.5 * (_u[_qp] * _normals[_qp] * _grad_test[_i][_qp] + (_normals[_qp] * _grad_u[_qp]) * _test[_i][_qp]);
  r += 0.5 * (_u_neighbor[_qp] * _normals[_qp] * _grad_test[_i][_qp] - _test[_i][_qp] * _normals[_qp] * _grad_u[_qp]);

  r += _s * (_u[_qp] - _u_neighbor[_qp]) * _test[_i][_qp];

//  r -= (0.5 * (_grad_u[_qp] + _grad_u_neighbor[_qp]) * _normals[_qp])  * (_test[_i][_qp] - _test_neighbor[_i][_qp]);

//  r -= (0.5 * (_grad_u[_qp] + _grad_u_neighbor[_qp]) * _normals[_qp] * _test[_i][_qp]);
//  r += _e * (0.5 * _grad_test[_i][_qp] * _normals[_qp] * (_u[_qp] - _u_neighbor[_qp]));
//  r += _s * _test[_i][_qp] * (_u[_qp] - _u_neighbor[_qp]);

//  printf("a = % lf: u = (% lf, % lf)", r, _u[_qp], _u_neighbor[_qp]);
//  printf("% lf, % lf, %lf | ", _grad_u[_qp](0), _grad_u[_qp](1), _grad_u[_qp](2));
//  printf("% lf, % lf, %lf", _grad_u_neighbor[_qp](0), _grad_u_neighbor[_qp](1), _grad_u_neighbor[_qp](2));
//  printf("\n");

//  Real r = 0.5 * _test[_i][_qp] * _grad_u[_qp] * _normals[_qp];

  return r;

//  return (0.5 * (_grad_u[_qp] + _grad_u_neighbor[_qp]) * _normals[_qp]) * (_test[_i][_qp] - _test_neighbor[_i][_qp])
//       - (0.5 * (_grad_test[_i][_qp] + _grad_test_neighbor[_i][_qp]) * _normals[_qp] * (_u[_qp] - _u_neighbor[_qp]));
}

