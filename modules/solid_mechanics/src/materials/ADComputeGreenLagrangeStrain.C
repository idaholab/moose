//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeGreenLagrangeStrain.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ADComputeGreenLagrangeStrain);

InputParameters
ADComputeGreenLagrangeStrain::validParams()
{
  InputParameters params = ADComputeStrainBase::validParams();
  params.addClassDescription("Compute a Green-Lagrange strain.");
  return params;
}

ADComputeGreenLagrangeStrain::ADComputeGreenLagrangeStrain(const InputParameters & parameters)
  : ADComputeStrainBase(parameters)
{
  // error out if unsupported features are to be used
  if (_global_strain)
    paramError("global_strain",
               "Global strain (periodicity) is not yet supported for Green-Lagrange strains");
  if (!_eigenstrains.empty())
    paramError("eigenstrain_names",
               "Eigenstrains are not yet supported for Green-Lagrange strains");
  if (_volumetric_locking_correction)
    paramError("volumetric_locking_correction",
               "Volumetric locking correction is not implemented for Green-Lagrange strains");
}

void
ADComputeGreenLagrangeStrain::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    auto dxu = ADRankTwoTensor::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);
    auto dxuT = dxu.transpose();

    _mechanical_strain[_qp] = _total_strain[_qp] = (dxuT + dxu + dxuT * dxu) / 2.0;
  }
}
