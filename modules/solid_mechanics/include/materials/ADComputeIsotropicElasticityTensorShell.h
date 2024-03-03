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

#define usingComputeIsotropicElasticityTensorShellMembers usingMaterialMembers

namespace libMesh
{
class QGauss;
}

class ADComputeIsotropicElasticityTensorShell : public Material
{
public:
  static InputParameters validParams();

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
  std::vector<ADMaterialProperty<RankFourTensor> *> _elasticity_tensor;

  /// Material property for ge matrix
  std::vector<const ADMaterialProperty<RankTwoTensor> *> _ge;
};
