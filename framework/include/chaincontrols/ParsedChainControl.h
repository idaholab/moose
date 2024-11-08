//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControl.h"
#include "MooseParsedFunctionBase.h"

class ChainControlParsedFunctionWrapper;

/**
 * Parses and evaluates a function expression to populate a control value.
 */
class ParsedChainControl : public ChainControl, public MooseParsedFunctionBase
{
public:
  static InputParameters validParams();

  ParsedChainControl(const InputParameters & parameters);

  virtual void init() override;
  virtual void execute() override;

protected:
  /**
   * Builds the function that will be evaluated by this control
   */
  void buildFunction();

  /// Function expression to parse and evaluate
  const std::string _function_expression;
  /// Control value to populate
  Real & _value;
  /// Pointer to the Parsed chain control function
  std::unique_ptr<ChainControlParsedFunctionWrapper> _function_ptr;
  /// Spatial point at which to evaluate the function
  const Point _point;
};
