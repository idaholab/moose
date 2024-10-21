//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class TwoPhaseNCGPartialPressureFluidProperties;

/**
 * Computes a property from a TwoPhaseNCGPartialPressureFluidProperties object.
 */
class TwoPhaseNCGPartialPressureFunction : public Function, public FunctionInterface
{
public:
  /// Number of expected arguments for each property call
  static const std::map<std::string, unsigned int> _n_expected_args;

  static InputParameters validParams();

  TwoPhaseNCGPartialPressureFunction(const InputParameters & parameters);

  virtual void initialSetup() override;
  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Fluid properties object
  const TwoPhaseNCGPartialPressureFluidProperties * _fp;
  /// Property call
  const MooseEnum & _property_call;
  /// Argument 1 function
  const Function & _arg1_fn;
  /// Argument 2 function
  const Function & _arg2_fn;
};
