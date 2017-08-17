/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMassPSPG.h"

template <>
InputParameters
validParams<INSMassPSPG>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription(
      "This class computes the PSPG stabilization components for the incompressibility equation.");
  return params;
}

INSMassPSPG::INSMassPSPG(const InputParameters & parameters) : INSBase(parameters) {}

Real
INSMassPSPG::computeQpResidual()
{
  // To do: Pick this in a more intelligent way
  Real alpha = 1;

  Real tau = alpha * _current_elem->hmax() * _current_elem->hmax() / (2. * _mu[_qp]);
  return tau * _grad_test[_i][_qp] * (computeStrongConvectiveTerm() + computeStrongViscousTerm() +
                                      computeStrongPressureTerm() + computeStrongGravityTerm());
}

Real
INSMassPSPG::computeQpJacobian()
{
  // To do
  return 0.0;
}

Real
INSMassPSPG::computeQpOffDiagJacobian(unsigned jvar)
{
  // To do
  return 0.0;
}
