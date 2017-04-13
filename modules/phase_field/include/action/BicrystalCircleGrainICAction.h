/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BICRYSTALCIRCLEGRAINICACTION_H
#define BICRYSTALCIRCLEGRAINICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Bicrystal with a circular grain and an embedding outer grain
 */
class BicrystalCircleGrainICAction : public Action
{
public:
  BicrystalCircleGrainICAction(const InputParameters & params);

  virtual void act();

private:
  const std::string _var_name_base;
  const unsigned int _op_num;

  const Real _radius;
  const Real _x, _y, _z;
  const Real _int_width;

  const bool _3D_sphere;
};

template <>
InputParameters validParams<BicrystalCircleGrainICAction>();

#endif // BICRYSTALCIRCLEGRAINICACTION_H
