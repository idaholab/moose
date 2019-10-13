//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "ADComputeIsotropicElasticityTensorShell.h"

#define usingComputeShellStressMembers usingMaterialMembers;

// Forward Declarations
template <ComputeStage>
class ADComputeShellStress;

namespace libMesh
{
class QGauss;
}

declareADValidParams(ADComputeShellStress);

template <ComputeStage compute_stage>
class ADComputeShellStress : public ADMaterial<compute_stage>
{
public:
  ADComputeShellStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Material property for strain increment
  std::vector<const ADMaterialProperty(RankTwoTensor) *> _strain_increment;

  /// Material property for current stress
  std::vector<ADMaterialProperty(RankTwoTensor) *> _stress;

  /// Material property for old stress
  std::vector<const MaterialProperty<RankTwoTensor> *> _stress_old;

  /// Material property for elasticity tensor
  std::vector<const ADMaterialProperty(RankFourTensor) *> _elasticity_tensor;

  /// Quadrature points along thickness
  std::vector<Point> _t_points;

  usingMaterialMembers;
};
