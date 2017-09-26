/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef INTERACTIONINTEGRALBENCHMARKBC_H
#define INTERACTIONINTEGRALBENCHMARKBC_H

#include "PresetNodalBC.h"
#include "CrackFrontDefinition.h"

// Forward Declarations
class InteractionIntegralBenchmarkBC;
class Function;

template <>
InputParameters validParams<InteractionIntegralBenchmarkBC>();
void addInteractionIntegralBenchmarkBCParams(InputParameters & params);

/**
 * Implements a boundary condition that enforces a displacement field around a
 * crack tip based on applied stress intensity factors KI, KII, and KIII. This
 * is used to test the interaction integral capability.
 */
class InteractionIntegralBenchmarkBC : public PresetNodalBC
{
public:
  InteractionIntegralBenchmarkBC(const InputParameters & parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual Real computeQpValue();

  const int _component;
  const CrackFrontDefinition * _crack_front_definition;
  const unsigned int _crack_front_point_index;

  Real _r;
  Real _theta;
  const Real _poissons_ratio;
  const Real _youngs_modulus;
  Real _kappa;
  Real _mu;
  Function & _ki_function;
  Function & _kii_function;
  Function & _kiii_function;
};

#endif // INTERACTIONINTEGRALBENCHMARKBC_H
