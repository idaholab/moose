//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SLKKSMultiPhaseBase.h"

/**
 * Enforce sum of phase sublattice concentrations to be the real concentration.
 * D. Schwen et al. https://doi.org/10.1016/j.commatsci.2021.110466
 *
 * \see SLKKSPhaseChemicalPotential
 */
class SLKKSMultiPhaseConcentration : public SLKKSMultiPhaseBase
{
public:
  static InputParameters validParams();

  SLKKSMultiPhaseConcentration(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Position of the nonlinear variable in the cs list
  int _l;

  ///@{ Switching functions for each phase and their derivatives w.r.t. all etas
  std::vector<const MaterialProperty<Real> *> _prop_h;
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_dhdeta;
  ///@}
};
