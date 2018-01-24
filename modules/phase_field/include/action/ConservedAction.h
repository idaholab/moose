/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSERVEDACTION_H
#define CONSERVEDACTION_H

// MOOSE includes
#include "Action.h"

#include "libmesh/fe_type.h"

// Forward declaration
class ConservedAction;

template <>
InputParameters validParams<ConservedAction>();

class ConservedAction : public Action
{
public:
  ConservedAction(const InputParameters & params);

  virtual void act() override;

protected:
  /// Type of solve
  enum class SolveType
  {
    DIRECT,
    REVERSE_SPLIT,
    FORWARD_SPLIT
  };
  /// Name of chemical potential variable for split solves
  std::string _chempot_name;
  /// Type of solve to use used in the action
  const SolveType _solve_type;
  /// Name of the variable being created
  const NonlinearVariableName _var_name;
  /// FEType for the variable being created
  FEType _fe_type;
  /// Scaling parameter
  const Real _scaling;
};

#endif // CONSERVEDACTION_H
