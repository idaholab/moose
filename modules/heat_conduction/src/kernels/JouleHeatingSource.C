#include "JouleHeatingSource.h"

template <>
InputParameters
validParams<JouleHeatingSource>()
{
  InputParameters params = validParams<HeatSource>();
  params.addCoupledVar("elec", "Electric potential for joule heating.");
  params.addCoupledVar("args", "Vector of arguments of the diffusivity");
  params.addParam<MaterialPropertyName>(
      "electrical_conductivity",
      "electrical_conductivity",
      "Material property providing electrical conductivity of the material.");
  return params;
}

JouleHeatingSource::JouleHeatingSource(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<HeatSource>>(parameters),
    _grad_elec(coupledGradient("elec")),
    _elec_var(coupled("elec")),
    _elec_cond(getMaterialProperty<Real>("electrical_conductivity")),
    _delec_cond_dT(getMaterialPropertyDerivative<Real>("electrical_conductivity", _var.name())),
    _delec_cond_darg(_coupled_moose_vars.size())
{
  for (unsigned int i = 0; i < _delec_cond_darg.size(); ++i)
    _delec_cond_darg[i] = &getMaterialPropertyDerivative<Real>("electrical_conductivity",
                                                               _coupled_moose_vars[i]->name());
}

void
JouleHeatingSource::initialSetup()
{
  validateNonlinearCoupling<Real>("electrical_conductivity");
}

Real
JouleHeatingSource::computeQpResidual()
{
  return -_elec_cond[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _test[_i][_qp];
}

Real
JouleHeatingSource::computeQpJacobian()
{
  return -_delec_cond_dT[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
JouleHeatingSource::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _elec_var)
    return -2 * _elec_cond[_qp] * _grad_elec[_qp] * _grad_phi[_j][_qp] * _test[_i][_qp] -
           (*_delec_cond_darg[cvar])[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] *
               _test[_i][_qp];

  return -(*_delec_cond_darg[cvar])[_qp] * _grad_elec[_qp] * _grad_elec[_qp] * _phi[_j][_qp] *
         _test[_i][_qp];
}
