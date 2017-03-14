/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALVORONOIVOIDICACTION_H
#define POLYCRYSTALVORONOIVOIDICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Sets up a polycrystal initial condition with voids on grain boundaries for
 * all order parameters.
 */
class PolycrystalVoronoiVoidICAction : public Action
{
public:
  PolycrystalVoronoiVoidICAction(const InputParameters & params);

  virtual void act();

protected:
  const unsigned int _op_num;
  const std::string _var_name_base;
};

template <>
InputParameters validParams<PolycrystalVoronoiVoidICAction>();

#endif // POLYCRYSTALVORONOIVOIDICACTION_H
