/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TRICRYSTAL2CIRCLEGRAINSICACTION_H
#define TRICRYSTAL2CIRCLEGRAINSICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables to model a polycrystal with op_num orderparameters
 */
class Tricrystal2CircleGrainsICAction : public Action
{
public:
  Tricrystal2CircleGrainsICAction(const InputParameters & params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  std::string _var_name_base;
  unsigned int _op_num;
};

template <>
InputParameters validParams<Tricrystal2CircleGrainsICAction>();

#endif // TRICRYSTAL2CIRCLEGRAINSICACTION_H
