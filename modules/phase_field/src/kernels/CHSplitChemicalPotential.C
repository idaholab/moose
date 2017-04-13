/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHSplitChemicalPotential.h"

template <>
InputParameters
validParams<CHSplitChemicalPotential>()
{
  InputParameters params = validParams<Kernel>();
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
        getMaterialPropertyDerivative<Real>(_mu_prop_name, getVar("c", 0)->name())),
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
