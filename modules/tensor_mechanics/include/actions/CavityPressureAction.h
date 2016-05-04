/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAVITYPRESSUREACTION_H
#define CAVITYPRESSUREACTION_H

#include "Action.h"
#include "MooseTypes.h"

class CavityPressureAction;

template<>
InputParameters validParams<CavityPressureAction>();

class CavityPressureAction: public Action
{
public:
  CavityPressureAction(const InputParameters & params);

  virtual void act();

private:
  const std::vector<BoundaryName> _boundary;

protected:
  bool _use_displaced_mesh;
};

#endif // CAVITYPRESSUREACTION_H
