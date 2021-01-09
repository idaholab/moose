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

InputParameters
ComputeSmallStrainNOSPD::validParams()
{
  InputParameters params = ComputeStrainBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for computing nodal quantities for the residual and Jacobian calculation "
      "for the peridynamic correspondence models under small strain assumptions");

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

  // zero out all strain measures for broken bond
  if (_bond_status_var->getElementalValue(_current_elem) < 0.5)
  {
    _mechanical_strain[_qp].zero();
    _total_strain[_qp].zero();
  }
}

void
ComputeSmallStrainNOSPD::computeQpTotalStrain()
{
  // the small strain tensor
  _total_strain[_qp] = 0.5 * (_deformation_gradient[_qp].transpose() + _deformation_gradient[_qp]) -
                       RankTwoTensor(RankTwoTensor::initIdentity);
}
