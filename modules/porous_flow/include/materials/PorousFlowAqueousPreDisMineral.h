//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWMASSFRACTIONAQUEOUSPREDISMINERAL_H
#define POROUSFLOWMASSFRACTIONAQUEOUSPREDISMINERAL_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowAqueousPreDisMineral;

template <>
InputParameters validParams<PorousFlowAqueousPreDisMineral>();

/**
 * Material designed to form a std::vector
 * of mass fractions of mineral concentrations from reaction rates
 * for an equilibrium precipitation-dissolution chemistry reaction system
 */
class PorousFlowAqueousPreDisMineral : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowAqueousPreDisMineral(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  /// number of equations in the aqueous geochemistry system
  const unsigned int _num_reactions;

  /// Mineral concentrations at quadpoint or nodes
  MaterialProperty<std::vector<Real>> & _sec_conc;

  /// porosity
  const MaterialProperty<Real> & _porosity_old;

  // old values of the mineral species concentrations
  const MaterialProperty<std::vector<Real>> & _sec_conc_old;

  // reaction rate of mineralisation
  const MaterialProperty<std::vector<Real>> & _reaction_rate;

  // whether the initial values of the secondary species concentrations have been supplied by the
  // user
  const bool _initial_conc_supplied;

  // number of secondary species concentrations supplied by the user
  const unsigned _num_initial_conc;

  // initial values of the secondary species concentrations
  std::vector<const VariableValue *> _initial_conc;
};

#endif // POROUSFLOWMASSFRACTIONAQUEOUSPREDISMINERAL_H
