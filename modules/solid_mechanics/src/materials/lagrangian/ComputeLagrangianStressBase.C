//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressBase.h"

InputParameters
ComputeLagrangianStressBase::validParams()
{
  InputParameters params = Material::validParams();

  params.addParam<bool>("large_kinematics", false, "Use a large displacement stress update.");

  params.addParam<std::string>("base_name", "Material property base name");

  return params;
}

ComputeLagrangianStressBase::ComputeLagrangianStressBase(const InputParameters & parameters)
  : Material(parameters),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _cauchy_stress(declareProperty<RankTwoTensor>(_base_name + "cauchy_stress")),
    _cauchy_jacobian(declareProperty<RankFourTensor>(_base_name + "cauchy_jacobian")),
    _pk1_stress(declareProperty<RankTwoTensor>(_base_name + "pk1_stress")),
    _pk1_jacobian(declareProperty<RankFourTensor>(_base_name + "pk1_jacobian")),
    _pk1_jacobian_bypass_fbar(
        declareProperty<RankFourTensor>(_base_name + "pk1_jacobian_bypass_fbar")),
    _dpk1_d_grad_u(declareProperty<RankFourTensor>(_base_name + "dpk1_d_grad_u")),
    _d_F_d_grad_u(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_deformation_gradient_d_grad_displacement")),
    _F_ust(getMaterialPropertyByName<RankTwoTensor>(_base_name +
                                                    "unstabilized_deformation_gradient")),
    _d_spatial_velocity_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_spatial_velocity_increment_d_deformation_gradient")),
    _d_F_stab_d_F_ust(
        getMaterialPropertyByName<RankFourTensor>(_base_name + "d_F_stab_d_F_unstabilized"))
{
}

void
ComputeLagrangianStressBase::initQpStatefulProperties()
{
  // Actually no need to zero out the stresses as they aren't stateful (yet)
}

void
ComputeLagrangianStressBase::computeQpProperties()
{
  computeQpStressUpdate();
  // Chain the kinematic policy into the PK1 Jacobian so the TL kernel consumes a single
  // d(PK1)/d(grad u) property and doesn't need to know about the generalized-alpha weighting.
  // For alpha = 1 (default) this is _pk1_jacobian itself.
  _dpk1_d_grad_u[_qp] = _pk1_jacobian[_qp] * _d_F_d_grad_u[_qp];
}
