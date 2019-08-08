//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeMultipleInelasticStress.h"

class ComputeMultiplePorousInelasticStress;

template <>
InputParameters validParams<ComputeMultiplePorousInelasticStress>();

/**
 * Compute state (stress and internal parameters such as plastic
 * strains and internal parameters) using an iterative process. A porosity material property
 * is defined and is calcuated from the trace of inelastic strain increment.
 */

class ComputeMultiplePorousInelasticStress : public ComputeMultipleInelasticStress
{
public:
  ComputeMultiplePorousInelasticStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  ///@{ Material property for porosity
  MaterialProperty<Real> & _porosity;
  const MaterialProperty<Real> & _porosity_old;
  ///@}

  /// Initial porosity value. Must be greater than zero.
  const Real _initial_porosity;
};
