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
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensorForward.h"

/**
 * EshelbyTensor defines a strain increment and rotation increment, for finite strains.
 */
class EshelbyTensor : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  EshelbyTensor(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

protected:
  /// Base name of the material system
  const std::string _base_name;

  const MaterialProperty<Real> & _sed;
  MaterialProperty<RankTwoTensor> & _eshelby_tensor;

  /// The stress tensor
  const MaterialProperty<RankTwoTensor> & _stress;

  /// The odl stress tensor
  const MaterialProperty<RankTwoTensor> & _stress_old;

  std::vector<const VariableGradient *> _grad_disp;

  MaterialProperty<RealVectorValue> & _J_thermal_term_vec;
  const VariableGradient & _grad_temp;
  const bool _has_temp;
  const MaterialProperty<RankTwoTensor> * const _total_deigenstrain_dT;
};
