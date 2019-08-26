#pragma once

#include "TestAction.h"

class ClosureTestAction;

template <>
InputParameters validParams<ClosureTestAction>();

/**
 * Action for setting up a closure test for 2-phase flow
 */
class ClosureTestAction : public TestAction
{
public:
  ClosureTestAction(InputParameters params);

protected:
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addOutput() override;

  /// Name of the dummy variable that is solved for
  const VariableName _dummy_name;
  /// Wall temperature
  const VariableName _T_wall_name;

  /// True if T_wall was specified
  bool _has_T_wall;
  /// Wall temperature function name
  const FunctionName _T_wall_fn;
  /// True if q_wall was specified
  bool _has_q_wall;
  /// Convective wall heat flux
  const Real & _q_wall;
  /// List of material properties to output
  const std::vector<std::string> & _output_properties;
};
