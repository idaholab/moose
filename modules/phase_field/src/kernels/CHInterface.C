/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHInterface.h"

template<>
InputParameters validParams<CHInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Gradient energy Cahn-Hilliard Kernel");
  params.addRequiredParam<std::string>("kappa_name", "The kappa used with the kernel");
  params.addRequiredParam<std::string>("mob_name", "The mobility used with the kernel");
  params.addParam<std::string>("Dmob_name", "DM", "The D mobility used with the kernel");
  params.addRequiredParam<std::string>("grad_mob_name", "The gradient of the mobility used with the kernel");
  params.addParam<bool>("has_MJac", false, "Jacobian information for the mobility is defined");

  return params;
}

CHInterface::CHInterface(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _kappa_name(getParam<std::string>("kappa_name")),
    _mob_name(getParam<std::string>("mob_name")),
    _Dmob_name(getParam<std::string>("Dmob_name")),
    _grad_mob_name(getParam<std::string>("grad_mob_name")),
    _kappa(getMaterialProperty<Real>(_kappa_name)),
    _M(getMaterialProperty<Real>(_mob_name)),
    _has_MJac(getParam<bool>("has_MJac")),
    _DM(_has_MJac ? &getMaterialProperty<Real>(_Dmob_name) : NULL),
    _grad_M(getMaterialProperty<RealGradient>(_grad_mob_name)),
    _Dgrad_Mnp(_has_MJac ? &getMaterialProperty<RealGradient>("Dgrad_Mnp") : NULL),
    _Dgrad_Mngp(_has_MJac ? &getMaterialProperty<Real>("Dgrad_Mngp") : NULL),
    _second_u(second()),
    _second_test(secondTest()),
    _second_phi(secondPhi())
{
}

Real
CHInterface::computeQpResidual()
{
  return _kappa[_qp] * _second_u[_qp].tr() * (_M[_qp] * _second_test[_i][_qp].tr() + _grad_M[_qp] * _grad_test[_i][_qp]);
}

Real
CHInterface::computeQpJacobian()
{
  //Actual value to return
  Real value = 0.0;

  if (isImplicit())
  {
    value = _kappa[_qp] * _second_phi[_j][_qp].tr() * (_M[_qp] * _second_test[_i][_qp].tr() + _grad_M[_qp]*_grad_test[_i][_qp]);

    if (_has_MJac)
    {
      RealGradient full_Dgrad_M = _grad_phi[_j][_qp] * (*_Dgrad_Mngp)[_qp] + _phi[_j][_qp] * (*_Dgrad_Mnp)[_qp];
      value += _kappa[_qp] * _second_u[_qp].tr() * ((*_DM)[_qp] * _phi[_j][_qp] * _second_test[_i][_qp].tr() + full_Dgrad_M * _grad_test[_i][_qp]);
    }
  }

  return value;
}
