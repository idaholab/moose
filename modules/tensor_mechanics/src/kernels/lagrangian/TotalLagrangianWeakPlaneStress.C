//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalLagrangianWeakPlaneStress.h"

registerMooseObject("TensorMechanicsApp", TotalLagrangianWeakPlaneStress);

InputParameters
TotalLagrangianWeakPlaneStress::validParams()
{
  InputParameters params = TotalLagrangianStressDivergence::validParams();
  params.addClassDescription("Plane stress kernel to provide out-of-plane strain contribution.");
  params.set<unsigned int>("component") = 0;
  params.suppressParameter<unsigned int>("component");
  params.suppressParameter<std::vector<VariableName>>("temperature");
  params.suppressParameter<std::vector<MaterialPropertyName>>("eigenstrain_names");
  params.suppressParameter<std::vector<VariableName>>("out_of_plane_strain");
  return params;
}

TotalLagrangianWeakPlaneStress::TotalLagrangianWeakPlaneStress(const InputParameters & parameters)
  : TotalLagrangianStressDivergence(parameters)
{
}

Real
TotalLagrangianWeakPlaneStress::computeQpResidual()
{
  return _test[_i][_qp] * _pk1[_qp](2, 2);
}

Real
TotalLagrangianWeakPlaneStress::computeQpJacobian()
{
  return _test[_i][_qp] * _dpk1[_qp](2, 2, 2, 2) * _phi[_j][_qp];
}

Real
TotalLagrangianWeakPlaneStress::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (auto beta : make_range(_ndisp))
    if (jvar == _disp_nums[beta])
      return _test[_i][_qp] * _dpk1[_qp].contractionIj(2, 2, gradTrial(beta));

  return 0;
}
