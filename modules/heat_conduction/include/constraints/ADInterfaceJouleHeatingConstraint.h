//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

/**
 * This Constraint implements thermal contact arising from
 * Joule heating at an interface subject to a potential drop.
 * This rough implementation follows the approach described in
 * Cincotti et al (2007) AIChE and Locci et al (2010) STAM
 */
class ADInterfaceJouleHeatingConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ADInterfaceJouleHeatingConstraint(const InputParameters & parameters);

protected:
  /**
   * Computes the heat source added to each interface side as a function of
   * the electric potential drop through Joule Heating
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) final;

  /// Lagrange multiplier variable from the separately solved electrical contact
  const ADVariableValue & _lm_electrical_potential;

  ///@{Electrical conductivity of the two solid materials at the closed gap interface
  const ADMaterialProperty<Real> & _primary_conductivity;
  const ADMaterialProperty<Real> & _secondary_conductivity;
  ///@}

  /**
   * Factor used to weight the distribution of the heat flux at interface
   * between the primary and secondary surfaces.
   */
  const Real _weight_factor;
};
