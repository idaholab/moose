//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeShellStress3D.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "ADComputeIsotropicElasticityTensorShell.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

registerMooseObject("TensorMechanicsApp", ADComputeShellStress3D);

InputParameters
ADComputeShellStress3D::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Compute in-plane stress using elasticity for shell");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");
  return params;
}

ADComputeShellStress3D::ADComputeShellStress3D(const InputParameters & parameters)
  : ADMaterial(parameters)
{
  // get number of quadrature points along thickness based on order
  std::unique_ptr<QGauss> t_qrule = std::make_unique<QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
  _t_points = t_qrule->get_points();
  _elasticity_tensor.resize(_t_points.size());
  _stress.resize(_t_points.size());
  _stress_old.resize(_t_points.size());
  _strain_increment.resize(_t_points.size());
  _rotation_matrix.resize(_t_points.size());
  _global_stress.resize(_t_points.size());
  for (unsigned int t = 0; t < _t_points.size(); ++t)
  {
    _elasticity_tensor[t] = &getADMaterialProperty<RankFourTensor>("t_points_" + std::to_string(t) +
                                                                   "_elasticity_tensor");
    _stress[t] = &declareADProperty<RankTwoTensor>("t_points_" + std::to_string(t) + "_stress");
    _stress_old[t] =
        &getMaterialPropertyOldByName<RankTwoTensor>("t_points_" + std::to_string(t) + "_stress");
    _strain_increment[t] = &getADMaterialProperty<RankTwoTensor>("t_points_" + std::to_string(t) +
                                                                 "_strain_increment");
    // rotation matrix and stress for output purposes only
    _rotation_matrix[t] = &getADMaterialProperty<RankTwoTensor>("t_points_" + std::to_string(t) +
                                                                "_rotation_increment");
    _global_stress[t] =
        &declareProperty<RankTwoTensor>("t_points_" + std::to_string(t) + "_global_stress");
  }
}

void
ADComputeShellStress3D::initQpStatefulProperties()
{
  // initialize stress tensor to zero
  for (unsigned int i = 0; i < _t_points.size(); ++i)
    (*_stress[i])[_qp].zero();
}

void
ADComputeShellStress3D::computeQpProperties()
{
  for (unsigned int i = 0; i < _t_points.size(); ++i)
  {
    (*_stress[i])[_qp] =
        (*_stress_old[i])[_qp] + (*_elasticity_tensor[i])[_qp] * (*_strain_increment[i])[_qp];

    for (unsigned int ii = 0; ii < 3; ++ii)
      for (unsigned int jj = 0; jj < 3; ++jj)
        _unrotated_stress(ii, jj) = MetaPhysicL::raw_value((*_stress[i])[_qp](ii, jj));
    (*_global_stress[i])[_qp] = MetaPhysicL::raw_value((*_rotation_matrix[i])[_qp].transpose()) *
                                _unrotated_stress *
                                MetaPhysicL::raw_value((*_rotation_matrix[i])[_qp]);
  }
}
