//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelBase.h"

class MortarContactPressureProvider;

/**
 * Gap flux model used to compute the conductance across a closed gap along which
 * two solid materials are in contact. This class computes the closed gap
 * conductance as a function of the mechanical normal contact pressure. The pressure is obtained
 * from a mortar contact user object when one is given, and otherwise from a coupled variable that
 * holds a prescribed pressure.
 */
class GapFluxModelPressureDependentConduction : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  GapFluxModelPressureDependentConduction(const InputParameters & parameters);

  ADReal computeFlux() const override;

protected:
  ///@{Temperatures from the primary and secondary surfaces at the interface
  const ADVariableValue & _primary_T;
  const ADVariableValue & _secondary_T;
  ///@}

  /// The contact pressure at the current quadrature point
  ADReal contactPressure() const;

  /// Mortar contact object that provides the physical contact pressure, or nullptr
  const MortarContactPressureProvider * const _contact_pressure_provider;

  /// Prescribed contact pressure variable, or nullptr when the pressure comes from the user object
  /// provider
  const ADVariableValue * const _contact_pressure;

  /// Parameter used to scale the closed gap interface conductance value
  const Real _scaling;

  ///@{Thermal conductivity of the two solid materials at the closed gap interface
  const ADMaterialProperty<Real> & _primary_conductivity;
  const ADMaterialProperty<Real> & _secondary_conductivity;
  ///@}

  ///@{Material hardness value of the two solid materials at the interface
  const ADMaterialProperty<Real> & _primary_hardness;
  const ADMaterialProperty<Real> & _secondary_hardness;
  ///@}
};
