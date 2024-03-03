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
#include "MooseTypes.h"

/**
 * EshelbyTensor defines a strain increment and rotation increment, for finite strains.
 */
template <bool is_ad>
class EshelbyTensorTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  EshelbyTensorTempl(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

protected:
  /// Base name of the material system
  const std::string _base_name;

  /// Whether to also compute Eshelby tensor's dissipation for C(t) integral
  const bool _compute_dissipation;

  const MaterialProperty<Real> & _sed;
  const MaterialProperty<Real> * _serd;

  MaterialProperty<RankTwoTensor> & _eshelby_tensor;
  MaterialProperty<RankTwoTensor> * _eshelby_tensor_dissipation;

  /// The stress tensor
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _stress;

  /// The old stress tensor
  const MaterialProperty<RankTwoTensor> & _stress_old;

  std::vector<const VariableGradient *> _grad_disp;
  std::vector<const VariableGradient *> _grad_disp_old;

  MaterialProperty<RealVectorValue> & _J_thermal_term_vec;
  const VariableGradient & _grad_temp;
  const bool _has_temp;
  const OptionalMaterialProperty<RankTwoTensor> & _total_deigenstrain_dT;
};

typedef EshelbyTensorTempl<false> EshelbyTensor;
typedef EshelbyTensorTempl<true> ADEshelbyTensor;
