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
class PolycrystalRandomICAction: public Action
{
public:
  PolycrystalRandomICAction(const InputParameters & params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  unsigned int _op_num;
  //unsigned int _grain_num;
  std::string _var_name_base;
  MooseEnum _random_type;
};

template<>
InputParameters validParams<PolycrystalRandomICAction>();

#endif //POLYCRYSTALRANDOMICACTION_H
