//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeGlobalTractionSmallStrain.h"

registerMooseObject("TensorMechanicsApp", CZMComputeGlobalTractionSmallStrain);

InputParameters
CZMComputeGlobalTractionSmallStrain::validParams()
{
  InputParameters params = CZMComputeGlobalTractionBase::validParams();

  params.addClassDescription(
      "Computes the czm traction in global coordinates for a small strain kinematic formulation");
  return params;
}

CZMComputeGlobalTractionSmallStrain::CZMComputeGlobalTractionSmallStrain(
    const InputParameters & parameters)
  : CZMComputeGlobalTractionBase(parameters)
{
}

void
CZMComputeGlobalTractionSmallStrain::computeEquilibriumTracionAndDerivatives()
{
  _traction_global[_qp] = _czm_total_rotation[_qp] * _interface_traction[_qp];
  _dtraction_djump_global[_qp] = _czm_total_rotation[_qp] * _dinterface_traction_djump[_qp] *
                                 _czm_total_rotation[_qp].transpose();
}
