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
  params.addParam<FunctionName>("f_shape", "1.0", "The shape function found in denominator");
  return params;
}

SoretDiffusion::SoretDiffusion(const InputParameters & parameters) :
    Kernel(parameters),
    _T_var(coupled("T")),
    _T(coupledValue("T")),
    _grad_T(coupledGradient("T")),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _D(getMaterialProperty<Real>("diff_name")),
    _Q(getMaterialProperty<Real>("Q_name")),
    _kb(8.617343e-5), // Boltzmann constant in eV/K
    _f_shape(&getFunction("f_shape"))
{
}

Real
SoretDiffusion::computeQpResidual()
{
  Real shape_value = _f_shape->value(_t, _q_point[_qp]);
  Real T_term = _D[_qp] * _Q[_qp] * _c[_qp] / (_kb * _T[_qp] * _T[_qp] * shape_value);

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
  {
    Real shape_value = _f_shape->value(_t, _q_point[_qp]);
    return _D[_qp] * _Q[_qp] * _c[_qp] * _grad_test[_i][_qp] *
           (_grad_phi[_j][_qp]/(_kb * _T[_qp] * _T[_qp] * shape_value) - 2.0 * _grad_T[_qp] *
           _phi[_j][_qp] / (_kb * _T[_qp] * _T[_qp] * _T[_qp] * shape_value));
  }

  return 0.0;
}

Real
SoretDiffusion::computeQpCJacobian()
{
  //Calculate the Jacobian for the c variable
  Real shape_value = _f_shape->value(_t, _q_point[_qp]);
  return _D[_qp] * _Q[_qp] * _phi[_j][_qp] * _grad_T[_qp] / (_kb * _T[_qp] * _T[_qp] * shape_value) * _grad_test[_i][_qp];
}

