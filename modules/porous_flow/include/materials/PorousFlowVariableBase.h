//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "PorousFlowMaterial.h"

/**
 * Base class for thermophysical variable materials, which assemble materials for
 * primary variables such as porepressure and saturation at the nodes
 * and quadpoints for all phases as required
 */
template <bool is_ad>
class PorousFlowVariableBaseTempl : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  static InputParameters validParams();

  PorousFlowVariableBaseTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Number of phases
  const unsigned int _num_phases;

  /// Number of components
  const unsigned int _num_components;

  /// Number of PorousFlow variables
  const unsigned int _num_pf_vars;

  /// Computed nodal or quadpoint values of porepressure of the phases
  GenericMaterialProperty<std::vector<Real>, is_ad> & _porepressure;

  /// d(porepressure)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real>>> * const _dporepressure_dvar;

  /// Grad(p) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _gradp_qp;

  /// d(grad porepressure)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<Real>>> * const _dgradp_qp_dgradv;

  /// d(grad porepressure)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgradp_qp_dv;

  /// Computed nodal or qp saturation of the phases
  GenericMaterialProperty<std::vector<Real>, is_ad> & _saturation;

  /// d(saturation)/d(PorousFlow variable)
  MaterialProperty<std::vector<std::vector<Real>>> * const _dsaturation_dvar;

  /// Grad(s) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _grads_qp;

  /// d(grad saturation)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<Real>>> * const _dgrads_qp_dgradv;

  /// d(grad saturation)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgrads_qp_dv;
};

typedef PorousFlowVariableBaseTempl<false> PorousFlowVariableBase;
typedef PorousFlowVariableBaseTempl<true> ADPorousFlowVariableBase;
