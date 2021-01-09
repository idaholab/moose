//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsBaseNOSPD.h"

InputParameters
MechanicsBaseNOSPD::validParams()
{
  InputParameters params = MechanicsBasePD::validParams();
  params.addClassDescription("Base class for kernels of the stabilized non-ordinary "
                             "state-based peridynamic correspondence models");

  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names",
      "List of eigenstrains to be coupled in non-ordinary state-based mechanics kernels");

  return params;
}

MechanicsBaseNOSPD::MechanicsBaseNOSPD(const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _multi(getMaterialProperty<Real>("multi")),
    _stress(getMaterialProperty<RankTwoTensor>("stress")),
    _shape2(getMaterialProperty<RankTwoTensor>("rank_two_shape_tensor")),
    _dgrad(getMaterialProperty<RankTwoTensor>("deformation_gradient")),
    _ddgraddu(getMaterialProperty<RankTwoTensor>("ddeformation_gradient_du")),
    _ddgraddv(getMaterialProperty<RankTwoTensor>("ddeformation_gradient_dv")),
    _ddgraddw(getMaterialProperty<RankTwoTensor>("ddeformation_gradient_dw")),
    _Jacobian_mult(getMaterialProperty<RankFourTensor>("Jacobian_mult")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _deigenstrain_dT(_eigenstrain_names.size())
{
  if (_temp_coupled)
    for (unsigned int i = 0; i < _deigenstrain_dT.size(); ++i)
      _deigenstrain_dT[i] =
          &getMaterialPropertyDerivative<RankTwoTensor>(_eigenstrain_names[i], _temp_var->name());
}

RankTwoTensor
MechanicsBaseNOSPD::computeDSDU(unsigned int component, unsigned int nd)
{
  // compute the derivative of stress w.r.t the solution components for small strain
  RankTwoTensor dSdU;
  if (component == 0)
    dSdU = _Jacobian_mult[nd] * 0.5 * (_ddgraddu[nd].transpose() + _ddgraddu[nd]);
  else if (component == 1)
    dSdU = _Jacobian_mult[nd] * 0.5 * (_ddgraddv[nd].transpose() + _ddgraddv[nd]);
  else if (component == 2)
    dSdU = _Jacobian_mult[nd] * 0.5 * (_ddgraddw[nd].transpose() + _ddgraddw[nd]);

  return dSdU;
}
