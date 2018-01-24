/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MathFreeEnergy.h"

template <>
InputParameters
validParams<MathFreeEnergy>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Material that implements the math free energy and its derivatives: "
                             "\nF = 1/4(1 + c)^2*(1 - c)^2");
  params.addRequiredCoupledVar("c", "Concentration variable");
  return params;
}

MathFreeEnergy::MathFreeEnergy(const InputParameters & parameters)
  : DerivativeFunctionMaterialBase(parameters), _c(coupledValue("c")), _c_var(coupled("c"))
{
}

Real
MathFreeEnergy::computeF()
{
  return 1.0 / 4.0 * (1.0 + _c[_qp]) * (1.0 + _c[_qp]) * (1.0 - _c[_qp]) * (1.0 - _c[_qp]);
}

Real
MathFreeEnergy::computeDF(unsigned int j_var)
{
  if (j_var == _c_var) // Note that these checks are only really necessary when the material has
                       // more than one coupled variable
    return _c[_qp] * (_c[_qp] * _c[_qp] - 1.0);
  else
    return 0.0;
}

Real
MathFreeEnergy::computeD2F(unsigned int j_var, unsigned int k_var)
{
  if ((j_var == _c_var) && (k_var == _c_var))
    return 3 * _c[_qp] * _c[_qp] - 1.0;
  else
    return 0.0;
}

Real
MathFreeEnergy::computeD3F(unsigned int j_var, unsigned int k_var, unsigned int l_var)
{
  if ((j_var == _c_var) && (k_var == _c_var) && (l_var == _c_var))
    return 6 * _c[_qp];
  else
    return 0.0;
}
