//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TestAction.h"

/**
 * Action for setting up a closure test for 2-phase flow
 */
class ClosureTestAction : public TestAction
{
public:
  ClosureTestAction(const InputParameters & params);

protected:
  virtual void addInitialConditions() override;
  virtual void addSolutionVariables() override;
  virtual void addAuxVariables() override;
  virtual void addMaterials() override;
  virtual void addOutput() override;

  void setupOutput();
  void setupADOutput();

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
  /// List of AD material properties to output
  const std::vector<std::string> & _output_ad_properties;

public:
  static InputParameters validParams();
};
