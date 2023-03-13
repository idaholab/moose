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
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeGeneralStressBase is the direct base class for stress calculator materials that may
 * leverage quantities based on the displaced mesh (like the UMAT plugins) rather than solely using
 * strain tensors computed by separate MOOSE material objects (those classes should directly derive
 * from ComputeStressBase, which in turn directly derives from ComputeGeneralStressBase).
 */
class ComputeGeneralStressBase : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ComputeGeneralStressBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Compute the stress and store it in the _stress material property
   * for the current quadrature point
   **/
  virtual void computeQpStress() = 0;

  /// Base name prepended to all material property names to allow for
  /// multi-material systems
  const std::string _base_name;

  /// Mechanical strain material property
  const MaterialProperty<RankTwoTensor> & _mechanical_strain;
  /// Stress material property
  MaterialProperty<RankTwoTensor> & _stress;
  /// Elastic strain material property
  MaterialProperty<RankTwoTensor> & _elastic_strain;

  /// Extra stress tensor
  const MaterialProperty<RankTwoTensor> & _extra_stress;

  /// initial stress components
  std::vector<const Function *> _initial_stress_fcn;

  /// derivative of stress w.r.t. strain (_dstress_dstrain)
  MaterialProperty<RankFourTensor> & _Jacobian_mult;
};
