//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePlaneFiniteStrainNOSPD.h"

registerMooseObject("PeridynamicsApp", ComputePlaneFiniteStrainNOSPD);

InputParameters
ComputePlaneFiniteStrainNOSPD::validParams()
{
  InputParameters params = ComputeFiniteStrainNOSPD::validParams();
  params.addClassDescription(
      "Class for computing nodal quantities for residual and jacobian calculation "
      "for peridynamic correspondence models under planar finite strain "
      "assumptions");

  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar out-of-plane strain variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain",
                       "Nonlinear out-of-plane strain variable for plane stress condition");

  return params;
}

ComputePlaneFiniteStrainNOSPD::ComputePlaneFiniteStrainNOSPD(const InputParameters & parameters)
  : ComputeFiniteStrainNOSPD(parameters),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _scalar_out_of_plane_strain_old(_scalar_out_of_plane_strain_coupled
                                        ? coupledScalarValueOld("scalar_out_of_plane_strain")
                                        : _zero),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? coupledValue("out_of_plane_strain")
                                                      : _zero),
    _out_of_plane_strain_old(_out_of_plane_strain_coupled ? coupledValueOld("out_of_plane_strain")
                                                          : _zero)
{
}

void
ComputePlaneFiniteStrainNOSPD::computeQpFhat()
{
  _deformation_gradient[_qp](2, 2) = computeQpOutOfPlaneDeformationGradient();

  RankTwoTensor deformation_gradient_old = _deformation_gradient_old[_qp];
  deformation_gradient_old(2, 2) = computeQpOutOfPlaneDeformationGradientOld();

  // Incremental deformation gradient of current step w.r.t previous step:
  // _Fhat = deformation_gradient * inv(deformation_gradient_old)
  _Fhat[_qp] = _deformation_gradient[_qp] * deformation_gradient_old.inverse();
}

Real
ComputePlaneFiniteStrainNOSPD::computeQpOutOfPlaneDeformationGradient()
{
  // This is consistent with the approximation of stretch rate tensor
  // D = log(sqrt(Fhat^T * Fhat)) / dt

  if (_scalar_out_of_plane_strain_coupled)
    return std::exp(_scalar_out_of_plane_strain[0]);
  else
    return std::exp(_out_of_plane_strain[_qp]);
}

Real
ComputePlaneFiniteStrainNOSPD::computeQpOutOfPlaneDeformationGradientOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return std::exp(_scalar_out_of_plane_strain_old[0]);
  else
    return std::exp(_out_of_plane_strain_old[_qp]);
}
