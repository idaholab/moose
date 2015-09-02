/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "anisoACInterface2.h"
#include <cmath>

template<>
InputParameters validParams<anisoACInterface2>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Anisotropic Gradient energy Allen-Cahn Kernel Part 2");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic parameter");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

anisoACInterface2::anisoACInterface2(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >(name, parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _eps(getMaterialProperty<Real>("eps_name"))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);
  _depsdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
   { _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", _coupled_moose_vars[i]->name());
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", _coupled_moose_vars[i]->name());
 }}

Real _cos_AC2;
Real _angle_AC2;
Real _depsdop_AC2;


RealGradient
anisoACInterface2::precomputeQpResidual()
{
  // Set interfacial part of residual
  return  _eps[_qp] * _eps[_qp] *  _L[_qp] * _grad_u[_qp];
}

RealGradient
anisoACInterface2::precomputeQpJacobian()
{ 
  // Set the value in special situation
  if (_grad_u[_qp]*_grad_u[_qp] ==0 )
  _depsdop_AC2 = 0;
 
   else
 { // Define the angle between grad_u and x axis
   _cos_AC2 = _grad_u[_qp](0)/sqrt(_grad_u[_qp]*_grad_u[_qp]);
  _angle_AC2 =  acos(_cos_AC2);
  // Set the value in special situation
   if (_cos_AC2*_cos_AC2 == 1)
    _depsdop_AC2 = 0;
 else
  // Derivative of eps with respect to u
 _depsdop_AC2 = 6*0.01*0.04*(-1)*sin(6*(_angle_AC2-1.57)) * (-1)/sqrt(1-_cos_AC2*_cos_AC2) * (_grad_phi[_j][_qp](0) * sqrt(_grad_u[_qp]*_grad_u[_qp]) - _grad_u[_qp](0)*_grad_phi[_j][_qp]*_grad_u[_qp]/sqrt(_grad_u[_qp]*_grad_u[_qp]))/(_grad_u[_qp]*_grad_u[_qp]);
}


  // Set Jacobian using product rule
  return _L[_qp] * ( _eps[_qp]*_eps[_qp]* _grad_phi[_j][_qp] + 2*_eps[_qp]*_depsdop_AC2 * _phi[_j][_qp] * _grad_u[_qp]);
}

Real
anisoACInterface2::computeQpOffDiagJacobian(unsigned int jvar)
{


  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal jaocbian terms from mobility dependence
  return _L[_qp] * 2 *_eps[_qp] * (*_depsdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}
