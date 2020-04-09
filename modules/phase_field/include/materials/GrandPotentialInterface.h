//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Calculate Grand Potential interface parameters for a specified interfacial free energy and width.
 */
class GrandPotentialInterface : public Material
{
public:
  static InputParameters validParams();

  GrandPotentialInterface(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// list of interfacial free energies
  const std::vector<Real> _sigma;

  /// Interface width for the interface with the median interfacial free energy
  const Real _width;

  /// number of interface pairs
  const unsigned int _n_pair;

  ///@{ Calculated gamma parameter values
  std::vector<Real> _gamma;
  Real _kappa;
  Real _mu;
  ///@}

  /// gamma material property names
  std::vector<MaterialPropertyName> _gamma_name;

  ///@{ Material properties for all interface pairs
  std::vector<MaterialProperty<Real> *> _gamma_prop;
  MaterialProperty<Real> & _kappa_prop;
  MaterialProperty<Real> & _mu_prop;
  ////@}
};
