/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHANICSACTION_H
#define SOLIDMECHANICSACTION_H

#include "Action.h"
#include "MooseTypes.h"

class SolidMechanicsAction;

template <>
InputParameters validParams<SolidMechanicsAction>();

class SolidMechanicsAction : public Action
{
public:
  SolidMechanicsAction(const InputParameters & params);

  virtual void act();

private:
  const NonlinearVariableName _disp_x;
  const NonlinearVariableName _disp_y;
  const NonlinearVariableName _disp_z;
  const NonlinearVariableName _disp_r;
  const NonlinearVariableName _temp;
  const Real _zeta;
  const Real _alpha;
};

#endif
