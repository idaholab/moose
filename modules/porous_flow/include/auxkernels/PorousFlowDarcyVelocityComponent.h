//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "PorousFlowDictator.h"

/**
 * Computes a component of the Darcy velocity:
 * -k_ij * krel /mu (nabla_j P - w_j)
 * where k_ij is the permeability tensor,
 * krel is the relative permeaility,
 * mu is the fluid viscosity,
 * P is the fluid pressure
 * and w_j is the fluid weight
 * This is measured in m^3 . s^-1 . m^-2
 */
template <bool is_ad>
class PorousFlowDarcyVelocityComponentTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  PorousFlowDarcyVelocityComponentTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Relative permeability of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _relative_permeability;

  /// Viscosity of each component in each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_viscosity;

  /// Permeability of porous material
  const GenericMaterialProperty<RealTensorValue, is_ad> & _permeability;

  /// Gradient of the pore pressure in each phase
  const GenericMaterialProperty<std::vector<RealGradient>, is_ad> & _grad_p;

  /// Fluid density for each phase (at the qp)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density_qp;

  /// PorousFlowDicatator UserObject
  const PorousFlowDictator & _dictator;

  /// Index of the fluid phase
  const unsigned int _ph;

  /// Desired spatial component
  unsigned int _component;

  /// Gravitational acceleration
  const RealVectorValue _gravity;
};

typedef PorousFlowDarcyVelocityComponentTempl<false> PorousFlowDarcyVelocityComponent;
typedef PorousFlowDarcyVelocityComponentTempl<true> ADPorousFlowDarcyVelocityComponent;
