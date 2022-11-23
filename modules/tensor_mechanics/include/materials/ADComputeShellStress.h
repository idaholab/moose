//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "ADComputeIsotropicElasticityTensorShell.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"

#define usingComputeShellStressMembers usingMaterialMembers

namespace libMesh
{
class QGauss;
}

class ADComputeShellStress : public Material
{
public:
  static InputParameters validParams();

  ADComputeShellStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Material property for strain increment
  std::vector<const ADMaterialProperty<RankTwoTensor> *> _strain_increment;

  /// Material property for current stress
  std::vector<ADMaterialProperty<RankTwoTensor> *> _stress;

  /// Material property for old stress
  std::vector<const MaterialProperty<RankTwoTensor> *> _stress_old;

  /// Material property for elasticity tensor
  std::vector<const ADMaterialProperty<RankFourTensor> *> _elasticity_tensor;

  /// Quadrature points along thickness
  std::vector<Point> _t_points;

  /// Covariant base vector matrix material property to transform stress
  std::vector<const MaterialProperty<RankTwoTensor> *> _covariant_transformation_matrix;

  /// Global stress tensor material property
  std::vector<MaterialProperty<RankTwoTensor> *> _global_stress;

  /// Real value of stress in the local coordinate system
  RankTwoTensor _unrotated_stress;
};
