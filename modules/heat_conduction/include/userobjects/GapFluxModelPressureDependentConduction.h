//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelBase.h"

/**
 * Gap flux model used to compute the conductance across a closed gap along which
 * two solid materials are in contact. This class computes the closed gap
 * conductance as a function of the mechanical normal contact pressure. The normal
 * contact pressure is coupled as a Lagrange Multiplier in this mortar-based approach.
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

  /// Pressure (lagrange multiplier) variable
  const ADVariableValue & _contact_pressure;

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
