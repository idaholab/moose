/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSEXCAV
#define RICHARDSEXCAV

#include "NodalBC.h"

// Forward Declarations
class RichardsExcav;
class Function;

template <>
InputParameters validParams<RichardsExcav>();

/**
 * Allows specification of Dirichlet BCs on an evolving boundary
 * RichardsExcav is applied on a sideset, and the function
 * excav_geom_function tells moose where on the sideset
 * to apply the BC through the shouldApply() function
 */
class RichardsExcav : public NodalBC
{
public:
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
  Function & _func;
};

#endif // RICHARDSEXCAV
