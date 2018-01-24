/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINGROWTHACTION_H
#define GRAINGROWTHACTION_H

// MOOSE includes
#include "Action.h"

#include "libmesh/fe_type.h"

// Forward declaration
class GrainGrowthAction;

template <>
InputParameters validParams<GrainGrowthAction>();

class GrainGrowthAction : public Action
{
public:
  GrainGrowthAction(const InputParameters & params);

  virtual void act();

protected:
  /// number of variables and variable name base for variable creation
  const unsigned int _op_num;
  const std::string _var_name_base;

  /// FEType for the variable being created
  const FEType _fe_type;
};

#endif // GRAINGROWTHACTION_H
