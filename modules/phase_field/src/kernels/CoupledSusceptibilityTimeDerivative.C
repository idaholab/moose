/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledSusceptibilityTimeDerivative.h"

template <>
InputParameters
validParams<CoupledSusceptibilityTimeDerivative>()
{
  InputParameters params = validParams<CoupledTimeDerivative>();
  params.addClassDescription("A modified coupled time derivative Kernel that multiply the time "
                             "derivative of a coupled variable by a function of the variables");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Base name of the function F defined in a DerivativeParsedMaterial");
  params.addCoupledVar("args", "Vector of arguments of the susceptibility");
  return params;
}

CoupledSusceptibilityTimeDerivative::CoupledSusceptibilityTimeDerivative(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<CoupledTimeDerivative>>(parameters),
    _F(getMaterialProperty<Real>("f_name")),
    _dFdu(getMaterialPropertyDerivative<Real>("f_name", _var.name())),
    _dFdarg(_coupled_moose_vars.size())
{
  // fetch derivatives
  for (unsigned int i = 0; i < _dFdarg.size(); ++i)
    _dFdarg[i] = &getMaterialPropertyDerivative<Real>("f_name", _coupled_moose_vars[i]->name());
}

void
CoupledSusceptibilityTimeDerivative::initialSetup()
{
  validateNonlinearCoupling<Real>("f_name");
}

Real
CoupledSusceptibilityTimeDerivative::computeQpResidual()
{
  return CoupledTimeDerivative::computeQpResidual() * _F[_qp];
}

Real
CoupledSusceptibilityTimeDerivative::computeQpJacobian()
{
  return CoupledTimeDerivative::computeQpResidual() * _dFdu[_qp] * _phi[_j][_qp];
}

Real
CoupledSusceptibilityTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _v_var)
    return CoupledTimeDerivative::computeQpOffDiagJacobian(jvar) * _F[_qp] +
           CoupledTimeDerivative::computeQpResidual() * _phi[_j][_qp] * (*_dFdarg[cvar])[_qp];

  return CoupledTimeDerivative::computeQpResidual() * _phi[_j][_qp] * (*_dFdarg[cvar])[_qp];
}
