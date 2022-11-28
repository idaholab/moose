//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHSplitChemicalPotential.h"

registerMooseObject("PhaseFieldApp", CHSplitChemicalPotential);

InputParameters
CHSplitChemicalPotential::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Chemical potential kernel in Split Cahn-Hilliard that solves "
                             "chemical potential in a weak form");
  params.addRequiredParam<MaterialPropertyName>("chemical_potential_prop",
                                                "Chemical potential property name");
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

CHSplitChemicalPotential::CHSplitChemicalPotential(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _mu_prop_name(getParam<MaterialPropertyName>("chemical_potential_prop")),
    _chemical_potential(getMaterialProperty<Real>(_mu_prop_name)),
    _dchemical_potential_dc(
        getMaterialPropertyDerivative<Real>(_mu_prop_name, coupledName("c", 0))),
    _c_var(coupled("c"))
{
}

Real
CHSplitChemicalPotential::computeQpResidual()
{
  return _test[_i][_qp] * (_u[_qp] - _chemical_potential[_qp]);
}

Real
CHSplitChemicalPotential::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp];
}

Real
CHSplitChemicalPotential::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return -_test[_i][_qp] * _dchemical_potential_dc[_qp] * _phi[_j][_qp];
  else
    return 0.0;
}
