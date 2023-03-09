//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

/**
 * Material designed to form a std::vector<std::vector>
 * of mass fractions from the individual mass fraction variables
 */
template <bool is_ad>
class PorousFlowMassFractionTempl : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowMassFractionTempl(const InputParameters & parameters);

protected:
  /// Mass fraction matrix at quadpoint or nodes
  GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mass_frac;

  /// Gradient of the mass fraction matrix at the quad points
  MaterialProperty<std::vector<std::vector<RealGradient>>> * const _grad_mass_frac;

  /// Derivative of the mass fraction matrix with respect to the porous flow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_frac_dvar;

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Builds the mass-fraction variable matrix at the quad point
   * @param qp the quad point
   */
  void build_mass_frac(unsigned int qp);

  /**
   * Number of mass-fraction variables provided by the user
   * This needs to be _num_phases*(_num_components - 1), since the
   * mass fraction of the final component in each phase is
   * determined as 1 - sum_{components}(mass fraction of all other components in the phase)
   */
  const unsigned int _num_passed_mf_vars;

  /// The variable number of the mass-fraction variables
  std::vector<unsigned int> _mf_vars_num;

  /// The mass-fraction variables
  std::vector<const GenericVariableValue<is_ad> *> _mf_vars;

  /// The gradient of the mass-fraction variables
  std::vector<const VariableGradient *> _grad_mf_vars;
};

typedef PorousFlowMassFractionTempl<false> PorousFlowMassFraction;
typedef PorousFlowMassFractionTempl<true> ADPorousFlowMassFraction;
