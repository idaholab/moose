//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianWPSStrain.h"

registerMooseObject("TensorMechanicsApp", ComputeLagrangianWPSStrain);

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

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    _F[_qp](2, 2) += _out_of_plane_strain[_qp];
}
