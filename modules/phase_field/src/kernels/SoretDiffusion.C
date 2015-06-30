/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SoretDiffusion.h"
template<>
InputParameters validParams<SoretDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Add Soret effect to Split formulation Cahn-Hilliard Kernel");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addRequiredParam<MaterialPropertyName>("diff_name", "The diffusivity used with the kernel");
  params.addParam<MaterialPropertyName>("Q_name", "Qheat", "The material name for the heat of transport");
  return params;
}

SoretDiffusion::SoretDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _T_var(coupled("T")),
    _T(coupledValue("T")),
    _grad_T(coupledGradient("T")),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _D(getMaterialProperty<Real>("diff_name")),
    _Q(getMaterialProperty<Real>("Q_name")),
    _kb(8.617343e-5) // Boltzmann constant in eV/K
{
}

Real
SoretDiffusion::computeQpResidual()
{
  Real T_term = _D[_qp] * _Q[_qp] * _c[_qp] / (_kb * _T[_qp] * _T[_qp]);

  return T_term * _grad_T[_qp] * _grad_test[_i][_qp];
}

Real
SoretDiffusion::computeQpJacobian()
{
  if (_c_var == _var.number()) //Requires c jacobian
    return computeQpCJacobian();

  return 0.0;
}

Real
SoretDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_c_var == jvar) //Requires c jacobian
    return computeQpCJacobian();
  else if (_T_var == jvar) //Requires T jacobian
    return _D[_qp] * _Q[_qp] * _c[_qp] * _grad_test[_i][_qp] *
           (_grad_phi[_j][_qp]/(_kb * _T[_qp] * _T[_qp]) - 2.0 * _grad_T[_qp] * _phi[_j][_qp] / (_kb * _T[_qp] * _T[_qp] * _T[_qp]));

  return 0.0;
}

Real
SoretDiffusion::computeQpCJacobian()
{
  //Calculate the Jacobian for the c variable
  return _D[_qp] * _Q[_qp] * _phi[_j][_qp] * _grad_T[_qp] / (_kb * _T[_qp] * _T[_qp]) * _grad_test[_i][_qp];
}
