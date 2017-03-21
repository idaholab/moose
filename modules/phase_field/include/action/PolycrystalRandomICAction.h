/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALRANDOMICACTION_H
#define POLYCRYSTALRANDOMICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables to model a polycrystal with op_num orderparameters
 */
class PolycrystalRandomICAction : public Action
{
public:
  PolycrystalRandomICAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _op_num;
  const std::string _var_name_base;
  const MooseEnum _random_type;
};

template <>
InputParameters validParams<PolycrystalRandomICAction>();

#endif // POLYCRYSTALRANDOMICACTION_H
