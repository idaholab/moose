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
 * Material designed to form a std::vector
 * of mass fractions of mineral concentrations from reaction rates
 * for an equilibrium precipitation-dissolution chemistry reaction system
 */
class PorousFlowAqueousPreDisMineral : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowAqueousPreDisMineral(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  /// Number of equations in the aqueous geochemistry system
  const unsigned int _num_reactions;

  /// Aqueous phase number
  const unsigned int _aq_ph;

  /// Saturation
  const MaterialProperty<std::vector<Real>> & _saturation;

  /// Mineral concentrations at quadpoint or nodes
  MaterialProperty<std::vector<Real>> & _sec_conc;

  /// Porosity
  const MaterialProperty<Real> & _porosity_old;

  /// Old values of the mineral species concentrations
  const MaterialProperty<std::vector<Real>> & _sec_conc_old;

  /// Reaction rate of mineralisation
  const MaterialProperty<std::vector<Real>> & _reaction_rate;

  /// Whether the initial values of the secondary species concentrations have been supplied by the
  /// user
  const bool _initial_conc_supplied;

  /// Number of secondary species concentrations supplied by the user
  const unsigned _num_initial_conc;

  /// Initial values of the secondary species concentrations
  std::vector<const VariableValue *> _initial_conc;
};
