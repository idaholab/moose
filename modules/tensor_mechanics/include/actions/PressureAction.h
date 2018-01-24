/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PRESSUREACTION_H
#define PRESSUREACTION_H

#include "Action.h"

class PressureAction : public Action
{
public:
  PressureAction(const InputParameters & params);

  virtual void act() override;

protected:
  std::vector<std::vector<AuxVariableName>> _save_in_vars;
  std::vector<bool> _has_save_in_vars;
};

template <>
InputParameters validParams<PressureAction>();

#endif // PRESSUREACTION_H
