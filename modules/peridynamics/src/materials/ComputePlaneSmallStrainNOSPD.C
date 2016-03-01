//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePlaneSmallStrainNOSPD.h"

registerMooseObject("PeridynamicsApp", ComputePlaneSmallStrainNOSPD);

InputParameters
ComputePlaneSmallStrainNOSPD::validParams()
{
  InputParameters params = ComputeSmallStrainNOSPD::validParams();
  params.addClassDescription(
      "Class for computing nodal quantities for residual and jacobian calculation "
      "for peridynamic correspondence models under planar small strain assumptions");

  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar out-of-plane strain variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain",
                       "Nonlinear out-of-plane strain variable for plane stress condition");

  return params;
}

ComputePlaneSmallStrainNOSPD::ComputePlaneSmallStrainNOSPD(const InputParameters & parameters)
  : ComputeSmallStrainNOSPD(parameters),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? coupledValue("out_of_plane_strain") : _zero)
{
}

void
ComputePlaneSmallStrainNOSPD::computeQpTotalStrain()
{
  // the green-lagrange strain tensor
  _total_strain[_qp] = 0.5 * (_deformation_gradient[_qp].transpose() + _deformation_gradient[_qp]) -
                       RankTwoTensor(RankTwoTensor::initIdentity);
  _total_strain[_qp](2, 2) = computeOutOfPlaneStrain();
}

Real
ComputePlaneSmallStrainNOSPD::computeOutOfPlaneStrain()
{
  if (_scalar_out_of_plane_strain_coupled)
    return _scalar_out_of_plane_strain[0];
  else
    return _out_of_plane_strain[_qp];
}
