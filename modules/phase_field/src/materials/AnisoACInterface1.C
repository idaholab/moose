/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "anisoACInterface1.h"
#include <cmath>

template<>
InputParameters validParams<anisoACInterface1>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Anisotropic gradient energy Allen-Cahn Kernel Part 1");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("eps_name", "eps", "The anisotropic parameter");
  params.addParam<MaterialPropertyName>("eps1_name", "eps1", "The derivative of anisotropic parameter with respect to angle");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

anisoACInterface1::anisoACInterface1(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >(name, parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _eps(getMaterialProperty<Real>("eps_name")),
    _eps1(getMaterialProperty<Real>("eps1_name"))

{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);
  _depsdarg.resize(nvar);
  _deps1darg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
   { _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", _coupled_moose_vars[i]->name());
    _depsdarg[i] = &getMaterialPropertyDerivative<Real>("eps_name", _coupled_moose_vars[i]->name());
    _deps1darg[i] = &getMaterialPropertyDerivative<Real>("eps1_name", _coupled_moose_vars[i]->name());
 }}

RealVectorValue _vector;
RealVectorValue _vector1;

 
Real _exchange;
Real _exchange1;
Real _cos_AC1;
Real _depsdop_AC1;
Real _deps1dop_AC1;
Real _angle_AC1;

RealGradient
anisoACInterface1::precomputeQpResidual()
{ 
  // Set interfacial part of residual
   _vector = _eps[_qp]* _eps1[_qp]*  _L[_qp] * _grad_u[_qp];
   _exchange = -1*_vector(1);
   _vector(1) = _vector(0);
   _vector(0) = _exchange;
  
  return  _vector;
}

RealGradient
anisoACInterface1::precomputeQpJacobian()
{
 //Set the value in special situation
 if (_grad_u[_qp]*_grad_u[_qp]==0)
 {_depsdop_AC1 = 0;
  _deps1dop_AC1 = 0;}
 //Define the angle between grad_u and x aixs
 else
{ _cos_AC1 = _grad_u[_qp](0)/sqrt(_grad_u[_qp]*_grad_u[_qp]);
 _angle_AC1 =  acos(_cos_AC1);
 //Set the value in special situation
 if(_cos_AC1*_cos_AC1 == 1)
 {_depsdop_AC1 = 0;
  _deps1dop_AC1 = 0;}
 else
 {
  //Derivative of eps with respect to u
  _depsdop_AC1 = 6*0.01*0.04*(-1)*sin(6*(_angle_AC1-1.57)) * (-1)/sqrt(1-_cos_AC1*_cos_AC1) * (_grad_phi[_j][_qp](0) * sqrt(_grad_u[_qp]*_grad_u[_qp]) - _grad_u[_qp](0)*_grad_phi[_j][_qp]*_grad_u[_qp]/sqrt(_grad_u[_qp]*_grad_u[_qp]))/(_grad_u[_qp]*_grad_u[_qp]);
  //Derivative of eps1 with respect to u
 _deps1dop_AC1 = 36*0.01*0.04*(-1)*cos(6*(_angle_AC1-1.57)) * (-1)/sqrt(1-_cos_AC1*_cos_AC1) * (_grad_phi[_j][_qp](0) * sqrt(_grad_u[_qp]*_grad_u[_qp]) - _grad_u[_qp](0)*_grad_phi[_j][_qp]*_grad_u[_qp]/sqrt(_grad_u[_qp]*_grad_u[_qp]))/(_grad_u[_qp]*_grad_u[_qp]); 
}}
  //Set the Jacobian
  _vector1 = _L[_qp]* ( _eps[_qp]*_eps1[_qp]* _grad_phi[_j][_qp] + _depsdop_AC1 * _eps1[_qp] * _phi[_j][_qp] * _grad_u[_qp] + _deps1dop_AC1 * _eps[_qp] * _phi[_j][_qp] * _grad_u[_qp] );
   _exchange1 = -1*_vector1(1);
   _vector1(1) = _vector1(0);
   _vector1(0) = _exchange1;

  return  _vector1;
}

Real
anisoACInterface1::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal jaocbian terms from mobility dependence
   return _L[_qp] * (_eps1[_qp] * (*_depsdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp] + _eps[_qp] * (*_deps1darg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp]);



}
