#include "DGDiffusion.h"

template<>
InputParameters validParams<DGDiffusion>()
{
  InputParameters params = validParams<DGKernel>();
  // See header file for sigma and epsilon
  params.addRequiredParam<Real>("sigma", "sigma");
  params.addRequiredParam<Real>("epsilon", "epsilon");
  return params;
}

static
Real j(Real a, Real b)
{
  return a-b;
}

static
Real av(Real a, Real b)
{
  return 0.5 * (a+b);
}

DGDiffusion::DGDiffusion(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :DGKernel(name, moose_system, parameters),
   _epsilon(parameters.get<Real>("epsilon")),
   _sigma(parameters.get<Real>("sigma"))
{
}

Real
DGDiffusion::computeQpResidual()
{
  Real r = 0;

  const unsigned int elem_b_order = static_cast<unsigned int> (_fe->get_order());
  const double h_elem = _current_elem->volume()/_current_side_elem->volume() * 1./pow(elem_b_order, 2.);

  r -= av(_grad_u[_qp] * _normals[_qp], _grad_u_neighbor[_qp] * _normals[_qp]) * _test[_i][_qp];
  r += _epsilon * j(_u[_qp], _u_neighbor[_qp]) * 0.5 * _grad_test[_i][_qp] * _normals[_qp];

  r += _sigma/h_elem * j(_u[_qp], _u_neighbor[_qp]) * _test[_i][_qp];

  return r;
}

