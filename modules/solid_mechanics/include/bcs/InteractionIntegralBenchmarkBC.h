//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBCBase.h"
#include "CrackFrontDefinition.h"

// Forward Declarations
class Function;

void addInteractionIntegralBenchmarkBCParams(InputParameters & params);

/**
 * Implements a boundary condition that enforces a displacement field around a
 * crack tip based on applied stress intensity factors KI, KII, and KIII. This
 * is used to test the interaction integral capability.
 */
class InteractionIntegralBenchmarkBC : public DirichletBCBase
{
public:
  static InputParameters validParams();

  InteractionIntegralBenchmarkBC(const InputParameters & parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual Real computeQpValue();

  const int _component;
  const CrackFrontDefinition & _crack_front_definition;
  const unsigned int _crack_front_point_index;

  Real _r;
  Real _theta;
  const Real _poissons_ratio;
  const Real _youngs_modulus;
  Real _kappa;
  Real _mu;
  const Function & _ki_function;
  const Function & _kii_function;
  const Function & _kiii_function;
};
