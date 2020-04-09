//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

// Forward Declarations
class Function;

/**
 * Allows specification of Dirichlet BCs on an evolving boundary
 * RichardsExcav is applied on a sideset, and the function
 * excav_geom_function tells moose where on the sideset
 * to apply the BC through the shouldApply() function
 */
class RichardsExcav : public NodalBC
{
public:
  static InputParameters validParams();

  RichardsExcav(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  /**
   * if excav_geom_function is != 0 at the point on
   * the boundary then apply the dirichlet BC
   */
  virtual bool shouldApply();

  /**
   * The variable will be made equal to _p_excav
   * at the "active" points on the boundary
   */
  Real _p_excav;

  /**
   * Controls which points are "active" on the boundary
   * An "active" point is where _func != 0, and at
   * these points the Dirichlet condition variable = _p_excav
   * will be applied
   */
  const Function & _func;
};
