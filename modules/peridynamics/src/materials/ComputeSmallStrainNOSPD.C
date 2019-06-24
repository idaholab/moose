//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSmallStrainNOSPD.h"

registerMooseObject("PeridynamicsApp", ComputeSmallStrainNOSPD);

template <>
InputParameters
validParams<ComputeSmallStrainNOSPD>()
{
  InputParameters params = validParams<ComputeStrainBaseNOSPD>();
  params.addClassDescription(
      "Class for computing nodal quantities for residual and Jacobian calculation "
      "for Self-stabilized Non-Ordinary State-based PeriDynamic (SNOSPD) "
      "correspondence model under small strain assumptions");

  return params;
}

ComputeSmallStrainNOSPD::ComputeSmallStrainNOSPD(const InputParameters & parameters)
  : ComputeStrainBaseNOSPD(parameters)
{
}

void
ComputeSmallStrainNOSPD::computeQpStrain()
{
  computeQpDeformationGradient();

  computeQpTotalStrain();

  _mechanical_strain[_qp] = _total_strain[_qp];

  // Subtract Eigen strains
  for (auto es : _eigenstrains)
    _mechanical_strain[_qp] -= (*es)[_qp];
}

void
ComputeSmallStrainNOSPD::computeQpTotalStrain()
{
  // the green-lagrange strain tensor
  _total_strain[_qp] = 0.5 * (_deformation_gradient[_qp].transpose() * _deformation_gradient[_qp] -
                              RankTwoTensor(RankTwoTensor::initIdentity));
}
