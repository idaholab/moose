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

#define usingComputeIsotropicElasticityTensorShellMembers usingMaterialMembers;

// Forward Declarations
template <ComputeStage>
class ADComputeIsotropicElasticityTensorShell;

namespace libMesh
{
class QGauss;
}

declareADValidParams(ADComputeIsotropicElasticityTensorShell);

template <ComputeStage compute_stage>
class ADComputeIsotropicElasticityTensorShell : public ADMaterial<compute_stage>
{
public:
  ADComputeIsotropicElasticityTensorShell(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Lame' paramters
  Real _poissons_ratio;
  Real _shear_modulus;
  Real _youngs_modulus;

  /// Individual elasticity tensor
  RankFourTensor _Cijkl;

  /// Quadrature points along thickness
  std::vector<Point> _t_points;

  /// Material property elasticity tensor
  std::vector<ADMaterialProperty(RankFourTensor) *> _elasticity_tensor;

  /// Material property for ge matrix
  std::vector<const ADMaterialProperty(RankTwoTensor) *> _ge;

  usingMaterialMembers;
};
