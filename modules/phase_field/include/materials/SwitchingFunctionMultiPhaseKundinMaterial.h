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
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * SwitchingFunctionMultiPhaseKundinMaterial is a switching function for a general
 * multi-phase, multi-order parameter system with an arbitrary number of phases. Defined by
 * Kundin, Pogorelov, and Emmerich, Acta Mater., v 83, p.448-459 (2015), Eq. (4). For phase i,
 * the switching function is
 * \f$ g_i = \frac{\eta_i^2}{2} \left[ 15 (1-\eta_i) \left( 1 - \eta_i - \sum_{j \neq i}
 * \eta_j^2 \right) + \eta_i (5 - 3 \eta_i^2) \right] \f$
 * which reduces to the Folch-Plapp three-phase switching function in the case of three phases.
 */
template <bool is_ad>
class SwitchingFunctionMultiPhaseKundinMaterialTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  SwitchingFunctionMultiPhaseKundinMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Name of the function
  MaterialPropertyName _h_name;

  /// Order parameter for this phase (eta_i)
  const GenericVariableValue<is_ad> & _eta_i;
  const VariableName _eta_i_name;

  /// Order parameters for all phases (including this one)
  const unsigned int _num_eta;
  const std::vector<const GenericVariableValue<is_ad> *> _eta;
  const std::vector<VariableName> _eta_names;

  /// Index of eta_i within the list of all etas
  unsigned int _self_index;

  /// Switching function and derivatives
  GenericMaterialProperty<Real, is_ad> & _prop_h;
  std::vector<GenericMaterialProperty<Real, is_ad> *> _prop_dh;
  std::vector<std::vector<GenericMaterialProperty<Real, is_ad> *>> _prop_d2h;
};

typedef SwitchingFunctionMultiPhaseKundinMaterialTempl<false>
    SwitchingFunctionMultiPhaseKundinMaterial;
typedef SwitchingFunctionMultiPhaseKundinMaterialTempl<true>
    ADSwitchingFunctionMultiPhaseKundinMaterial;
