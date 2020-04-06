#pragma once

#include "THMControl.h"
#include "MooseParsedFunctionBase.h"

class THMParsedFunctionWrapper;

/**
 * This control block takes a parsed function and evaluates it
 */
class ParsedFunctionControl : public THMControl, public MooseParsedFunctionBase
{
public:
  ParsedFunctionControl(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void init() override;
  virtual void execute() override;

protected:
  /**
   * Build the function that will be evaluated by this control
   */
  void buildFunction();

  /// The user-defined function to be evaluated
  std::string _function;
  /// The function value
  Real & _value;
  /// Pointer to the Parsed function wrapper object
  std::unique_ptr<THMParsedFunctionWrapper> _function_ptr;

public:
  static InputParameters validParams();
};
