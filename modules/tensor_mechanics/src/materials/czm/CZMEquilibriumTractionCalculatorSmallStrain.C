//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMEquilibriumTractionCalculatorSmallStrain.h"

registerMooseObject("TensorMechanicsApp", CZMEquilibriumTractionCalculatorSmallStrain);

InputParameters
CZMEquilibriumTractionCalculatorSmallStrain::validParams()
{
  InputParameters params = CZMEquilibriumTractionCalculatorBase::validParams();

  params.addClassDescription(
      "Computes the czm traction in global coordinates for a small strain kinematic formulation");
  return params;
}

CZMEquilibriumTractionCalculatorSmallStrain::CZMEquilibriumTractionCalculatorSmallStrain(
    const InputParameters & parameters)
  : CZMEquilibriumTractionCalculatorBase(parameters)
{
}

void
CZMEquilibriumTractionCalculatorSmallStrain::computeEquilibriumTracionAndDerivatives()
{
  _traction_global[_qp] = _Q0[_qp] * _interface_traction[_qp];
  _dtraction_djump_global[_qp] = _Q0[_qp] * _dinterface_traction_djump[_qp] * _Q0[_qp].transpose();
}
