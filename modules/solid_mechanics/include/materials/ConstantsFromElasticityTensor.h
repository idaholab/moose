//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankFourTensorForward.h"
#include "GuaranteeConsumer.h"

/**
 * This material computes the elastic constants such as Young's modulus
 * from the elasticity tensor.
 */
class ConstantsFromElasticityTensor : public Material, public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  ConstantsFromElasticityTensor(const InputParameters & parameters);
  void initialSetup() override;

protected:
  void computeQpProperties() override;

private:
  /// Base name of the material system
  const std::string _base_name;

  ///{@ Elastic constants computed from tensor
  MaterialProperty<Real> & _youngs_modulus;
  MaterialProperty<Real> & _poissons_ratio;
  MaterialProperty<Real> & _shear_modulus;
  MaterialProperty<Real> & _bulk_modulus;
  ///@}

  /// Name of elasticity tensor
  const std::string _elasticity_tensor_name;

  /// The computed elastic constant
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
};
