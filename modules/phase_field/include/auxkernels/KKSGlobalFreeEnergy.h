//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalFreeEnergyBase.h"
#include "Material.h"

// Forward Declarations

/**
 * Compute the global free energy in the KKS Model
 * \f$ F = hF_a + (1-h)F_b + wg + \frac{\kappa}{2}|\eta|^2 \f$
 */
class KKSGlobalFreeEnergy : public TotalFreeEnergyBase
{
public:
  static InputParameters validParams();

  KKSGlobalFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const MaterialProperty<Real> & _prop_fa;
  const MaterialProperty<Real> & _prop_fb;
  const MaterialProperty<Real> & _prop_h;
  const MaterialProperty<Real> & _prop_g;

  /// Barrier term height
  const Real _w;

  /// Gradient interface free energy coefficients
  std::vector<const MaterialProperty<Real> *> _kappas;
};
