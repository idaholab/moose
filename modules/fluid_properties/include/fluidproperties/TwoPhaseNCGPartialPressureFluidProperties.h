//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoPhaseNCGFluidProperties.h"

/**
 * Two-phase fluid with single NCG using partial pressure mixture model
 */
class TwoPhaseNCGPartialPressureFluidProperties : public TwoPhaseNCGFluidProperties
{
public:
  static InputParameters validParams();

  TwoPhaseNCGPartialPressureFluidProperties(const InputParameters & parameters);

  /**
   * Computes the NCG mass fraction with the CG saturated at the given temperature
   */
  Real x_sat_ncg_from_p_T(Real p, Real T) const;

protected:
  /// NCG fluid properties
  const SinglePhaseFluidProperties & _fp_ncg;

  /// Primary vapor fluid properties
  const SinglePhaseFluidProperties * _fp_vapor_primary;

  /// Molar masses for each vapor in mixture
  std::vector<Real> _molar_masses;
};
