/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALCOLORINGICACTION_H
#define POLYCRYSTALCOLORINGICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Random Voronoi tesselation polycrystal action
 */
class PolycrystalColoringICAction : public Action
{
public:
  PolycrystalColoringICAction(const InputParameters & params);

  virtual void act() override;

private:
  const unsigned int _op_num;
  const std::string _var_name_base;
};

template <>
InputParameters validParams<PolycrystalColoringICAction>();

#endif // POLYCRYSTALCOLORINGICACTION_H
