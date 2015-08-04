/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALHEXGRAINICACTION_H
#define POLYCRYSTALHEXGRAINICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates ic for polycrystal hexagonal grain structure. Must have squared number of grains and periodic BCs.
 */
class PolycrystalHexGrainICAction: public Action
{
public:
  PolycrystalHexGrainICAction(const InputParameters & params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  std::string _var_name_base;
  unsigned int _op_num;
  unsigned int _grain_num;

  Real _x_offset;
  Real _perturbation_percent;
};

template<>
InputParameters validParams<PolycrystalHexGrainICAction>();

#endif //POLYCRYSTALHEXGRAINICACTION_H
