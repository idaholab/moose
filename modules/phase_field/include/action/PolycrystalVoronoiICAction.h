/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALVORONOIICACTION_H
#define POLYCRYSTALVORONOIICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Random Voronoi tesselation polycrystal action
 */
class PolycrystalVoronoiICAction : public Action
{
public:
  PolycrystalVoronoiICAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _op_num;
  const unsigned int _grain_num;
  const std::string _var_name_base;
};

template <>
InputParameters validParams<PolycrystalVoronoiICAction>();

#endif // POLYCRYSTALVORONOIICACTION_H
