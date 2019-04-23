/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef NodalGravity_H
#define NodalGravity_H

#include "NodalKernel.h"

// Forward Declarations
class NodalGravity;

template <>
InputParameters validParams<NodalGravity>();

/**
 * Calculates the gravitational force proportional to nodal mass
 */
class NodalGravity : public NodalKernel
{
public:
  NodalGravity(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Booleans for validity of params
  const bool _has_mass;
  const bool _has_nodal_mass_file;

  /// Mass associated with the node
  const Real _mass;

  /// HHT time integration parameter
  const Real _alpha;

  /// Acceleration due to gravity
  const Real _gravity_value;

  /// Time and space dependent factor multiplying acceleration due to gravity
  Function & _function;

  /// Map between boundary nodes and nodal mass
  std::map<dof_id_type, Real> _node_id_to_mass;
};

#endif /* NodalGravity_H */
