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
 * todo
 */

class ComputeMultiplePorousInelasticStress : public ComputeMultipleInelasticStress
{
public:
  ComputeMultiplePorousInelasticStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  MaterialProperty<Real> & _porosity;
  const MaterialProperty<Real> & _porosity_old;
  const Real _initial_porosity;
};
