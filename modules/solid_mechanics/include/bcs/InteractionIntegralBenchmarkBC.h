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

//Forward Declarations
class InteractionIntegralBenchmarkBC;
class Function;

template<>
InputParameters validParams<InteractionIntegralBenchmarkBC>();
void addInteractionIntegralBenchmarkBCParams(InputParameters& params);

/**
 * Implements a boundary condition that enforces a displacement field around a
 * crack tip based on applied stress intensity factors KI, KII, and KIII. This
 * is used to test the interaction integral capability.
 */
class InteractionIntegralBenchmarkBC : public PresetNodalBC
{
public:
  InteractionIntegralBenchmarkBC(const std::string & name, InputParameters parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual Real computeQpValue();

  const int _component;
  const CrackFrontDefinition * _crack_front_definition;
  const unsigned int _crack_front_node_index;

  Real _r;
  Real _theta;
  Real _poissons_ratio;
  Real _youngs_modulus;
  Real _kappa;
  Real _mu;
  Real _ki;
  Real _kii;
  Real _kiii;
};

#endif //INTERACTIONINTEGRALBENCHMARKBC_H
