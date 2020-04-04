//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHSplitConcentration.h"

registerMooseObject("PhaseFieldApp", CHSplitConcentration);

InputParameters
CHSplitConcentration::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Concentration kernel in Split Cahn-Hilliard that solves chemical potential in a weak form");
  params.addRequiredParam<MaterialPropertyName>("mobility", "Mobility property name");
  params.addRequiredCoupledVar("chemical_potential_var", "Chemical potential variable");
  return params;
}

CHSplitConcentration::CHSplitConcentration(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _mobility_name(getParam<MaterialPropertyName>("mobility")),
    _mobility(getMaterialProperty<RealTensorValue>(_mobility_name)),
    _dmobility_dc(getMaterialPropertyDerivative<RealTensorValue>(_mobility_name, name())),
    _mu_var(coupled("chemical_potential_var")),
    _grad_mu(coupledGradient("chemical_potential_var"))
{
}

Real
CHSplitConcentration::computeQpResidual()
{
  const RealVectorValue a = _mobility[_qp] * _grad_mu[_qp];
  return _grad_test[_i][_qp] * a;
}

Real
CHSplitConcentration::computeQpJacobian()
{
  const RealVectorValue a = _dmobility_dc[_qp] * _grad_mu[_qp];
  return _grad_test[_i][_qp] * a * _phi[_j][_qp];
}

Real
CHSplitConcentration::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _mu_var)
  {
    const RealVectorValue a = _mobility[_qp] * _grad_phi[_j][_qp];
    return _grad_test[_i][_qp] * a;
  }
  else
    return 0.0;
}
