//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPRESIDUALDEBUGACTION_H
#define SETUPRESIDUALDEBUGACTION_H

#include "Action.h"
#include "MooseTypes.h"

class SetupResidualDebugAction;

template <>
InputParameters validParams<SetupResidualDebugAction>();

/**
 *
 */
class SetupResidualDebugAction : public Action
{
public:
  SetupResidualDebugAction(InputParameters parameters);

  virtual void act() override;

protected:
  std::vector<NonlinearVariableName> _show_var_residual;
};

#endif /* SETUPRESIDUALDEBUGACTION_H */
