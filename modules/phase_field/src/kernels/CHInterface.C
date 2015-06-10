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
  params.addCoupledVar("args", "Vector of arguments to mobility");

  return params;
}

CHInterface::CHInterface(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<Kernel> >(name, parameters),
    _kappa_name(getParam<std::string>("kappa_name")),
    _mob_name(getParam<std::string>("mob_name")),
    _kappa(getMaterialProperty<Real>(_kappa_name)),
    _M(getMaterialProperty<Real>(_mob_name)),
    _dMdc(getMaterialPropertyDerivative<Real>(_mob_name, _var.name())),
    _d2Mdc2(getMaterialPropertyDerivative<Real>(_mob_name, _var.name(), _var.name())),
    _second_u(second()),
    _second_test(secondTest()),
    _second_phi(secondPhi()),
    _nvar(_coupled_moose_vars.size()),
    _dMdarg(_nvar),
    _d2Mdcdarg(_nvar),
    _d2Mdargdarg(_nvar),
    _coupled_grad_vars(_nvar)
{
  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    //Set material property values
    _dMdarg[i] = &getMaterialPropertyDerivative<Real>(_mob_name, _coupled_moose_vars[i]->name());
    _d2Mdcdarg[i] = &getMaterialPropertyDerivative<Real>(_mob_name, _var.name(), _coupled_moose_vars[i]->name());
    _d2Mdargdarg[i].resize(_nvar);
    for (unsigned int j = 0; j < _nvar; ++j)
      _d2Mdargdarg[i][j] = &getMaterialPropertyDerivative<Real>(_mob_name, _coupled_moose_vars[i]->name(), _coupled_moose_vars[j]->name());

    //Set coupled variable gradients
    _coupled_grad_vars[i] = &coupledGradient("args", i);
  }
}

Real
CHInterface::computeQpResidual()
{
  RealGradient grad_M = _dMdc[_qp]*_grad_u[_qp];
  for (unsigned int i = 0; i < _nvar; ++i)
    grad_M += (*_dMdarg[i])[_qp]*(*_coupled_grad_vars[i])[_qp];

  return _kappa[_qp]*_second_u[_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr()
                                          + grad_M*_grad_test[_i][_qp]);
}

Real
CHInterface::computeQpJacobian()
{
  // Set the gradient and gradient derivative values
  RealGradient grad_M = _dMdc[_qp]*_grad_u[_qp];

  RealGradient dgrad_Mdc = _d2Mdc2[_qp]*_phi[_j][_qp]*_grad_u[_qp]
                           + _dMdc[_qp]*_grad_phi[_j][_qp];

  for (unsigned int i = 0; i < _nvar; ++i)
  {
    grad_M += (*_dMdarg[i])[_qp]*(*_coupled_grad_vars[i])[_qp];
    dgrad_Mdc += (*_d2Mdcdarg[i])[_qp]*_phi[_j][_qp]*(*_coupled_grad_vars[i])[_qp];
  }

  //Jacobian value using product rule
  Real value = _kappa[_qp]*_second_phi[_j][_qp].tr()*(_M[_qp]*_second_test[_i][_qp].tr()
                                                      + grad_M*_grad_test[_i][_qp])
               + _kappa[_qp]*_second_u[_qp].tr()*(_dMdc[_qp]*_phi[_j][_qp]*_second_test[_i][_qp].tr()
                                                  + dgrad_Mdc*_grad_test[_i][_qp]);

  return value;
}

Real
CHInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set the gradient derivative
  RealGradient dgrad_Mdarg = (*_d2Mdcdarg[cvar])[_qp]*_phi[_j][_qp]*_grad_u[_qp] +
                             (*_dMdarg[cvar])[_qp]*_grad_phi[_j][_qp];

  for (unsigned int i = 0; i < _nvar; ++i)
    dgrad_Mdarg += (*_d2Mdargdarg[cvar][i])[_qp]*_phi[_j][_qp]*(*_coupled_grad_vars[cvar])[_qp];

  //Jacobian value using product rule
  Real value = _kappa[_qp]*_second_u[_qp].tr()*((*_dMdarg[cvar])[_qp]*_phi[_j][_qp]*_second_test[_i][_qp].tr()
                                                + dgrad_Mdarg*_grad_test[_i][_qp]);

  return value;

}
