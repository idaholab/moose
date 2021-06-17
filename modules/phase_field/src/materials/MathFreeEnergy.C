//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MathFreeEnergy.h"

registerMooseObject("PhaseFieldApp", MathFreeEnergy);

InputParameters
MathFreeEnergy::validParams()
{
  InputParameters params = DerivativeFunctionMaterialBase::validParams();
  params.addClassDescription("Material that implements the math free energy and its derivatives: "
                             "\n$F = 1/4(1 + c)^2(1 - c)^2$");
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
