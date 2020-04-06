//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"

// Forward Declarations

/**
 * Calculates the gravitational force proportional to nodal mass
 */
class NodalGravity : public NodalKernel
{
public:
  static InputParameters validParams();

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
  const Function & _function;

  /// Map between boundary nodes and nodal mass
  std::map<dof_id_type, Real> _node_id_to_mass;
};
