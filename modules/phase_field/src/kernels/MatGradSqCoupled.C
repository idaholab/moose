#include "MatGradSqCoupled.h"

template <>
InputParameters
validParams<MatGradSqCoupled>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("elec", "Electric field");
  params.addCoupledVar("args", "Additional variable");
  params.addParam<MaterialPropertyName>(
      "prefactor",
      "prefactor",
      "Material property providing a prefactor of electric field contribution");
  return params;
}

MatGradSqCoupled::MatGradSqCoupled(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _grad_elec(coupledGradient("elec")),
    _elec_var(coupled("elec")),
    _prefactor(getMaterialProperty<Real>("prefactor")),
    _dprefactor_dphi(getMaterialPropertyDerivative<Real>("prefactor", _var.name())),
    _dprefactor_darg(_coupled_moose_vars.size())
{
  for (unsigned int i = 0; i < _dprefactor_darg.size(); ++i)
    _dprefactor_darg[i] = &getMaterialPropertyDerivative<Real>("prefactor",
                                                               _coupled_moose_vars[i]->name());
}

void
MatGradSqCoupled::initialSetup()
{
  validateNonlinearCoupling<Real>("prefactor");
}

Real
MatGradSqCoupled::computeQpResidual()
{
  return -_prefactor[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _test[_i][_qp];
}

Real
MatGradSqCoupled::computeQpJacobian()
{
  return -_dprefactor_dphi[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
MatGradSqCoupled::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _elec_var)
    return -2 * _prefactor[_qp] * _grad_elec[_qp] * _grad_phi[_j][_qp] * _test[_i][_qp] -
           (*_dprefactor_darg[cvar])[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] *
               _test[_i][_qp];

  return -(*_dprefactor_darg[cvar])[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] *
         _test[_i][_qp];
}
