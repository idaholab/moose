/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NONCONSERVEDACTION_H
#define NONCONSERVEDACTION_H

// MOOSE includes
#include "Action.h"
#include "AddVariableAction.h"

#include "libmesh/fe_type.h"

// Forward declaration
class NonconservedAction;

template <>
InputParameters validParams<NonconservedAction>();

class NonconservedAction : public Action
{
public:
  NonconservedAction(const InputParameters & params);

  virtual void act();

protected:
  /// Name of the variable being created
  const NonlinearVariableName _var_name;
  /// FEType for the variable being created
  const FEType _fe_type;
};

#endif // NONCONSERVEDACTION_H
