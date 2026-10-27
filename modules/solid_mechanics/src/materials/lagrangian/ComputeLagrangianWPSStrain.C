//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianWPSStrain.h"

registerMooseObject("SolidMechanicsApp", ComputeLagrangianWPSStrain);

InputParameters
ComputeLagrangianWPSStrain::validParams()
{
  InputParameters params = ComputeLagrangianStrain::validParams();
  params.addRequiredCoupledVar("out_of_plane_strain", "The out-of-plane strain");
  return params;
}

ComputeLagrangianWPSStrain::ComputeLagrangianWPSStrain(const InputParameters & params)
  : ComputeLagrangianStrain(params), _out_of_plane_strain(coupledValue("out_of_plane_strain"))
{
}

void
ComputeLagrangianWPSStrain::computeDeformationGradient()
{
  ComputeLagrangianStrain::computeDeformationGradient();

  // Add the out-of-plane strain to BOTH the stabilized `_F` and the unstabilized
  // `_F_ust` AFTER F-bar has run. F-bar's partials (`_d_F_stab_d_F_*`) therefore do
  // NOT see this contribution -- strain_zz bypasses F-bar's non-local chain. The WPS
  // kernel and the TL kernel's `computeQpJacobianOutOfPlaneStrain` use the
  // "no-F-bar" pk1_jacobian variant to handle strain_zz perturbations correctly.
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _F[_qp](2, 2) += _out_of_plane_strain[_qp];
    _F_ust[_qp](2, 2) += _out_of_plane_strain[_qp];
  }
}
