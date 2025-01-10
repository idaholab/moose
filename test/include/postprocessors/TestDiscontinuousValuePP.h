//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

// Forward Declarations
class SolutionUserObjectBase;

/**
 * Compute the value of a variable or the gradient at a specified location.
 * Takes multivalued functions at boundaries into account
 */
class TestDiscontinuousValuePP : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestDiscontinuousValuePP(const InputParameters & parameters);

  virtual ~TestDiscontinuousValuePP(){};

  virtual void initialize() override{};

  virtual void execute() override{};

  virtual void finalize() override{};

  /// initialSetup gets the pointer to the solution UO
  virtual void initialSetup() override;

  virtual Real getValue() const override;

protected:
  /// The variable name from which a values is to be extracted
  const VariableName & _variable_name;

  /// The point to locate, stored as a vector for use with reinitElemPhys
  Point _point;

  /// This option allows to switch between value and gradient evaluation
  bool _evaluate_gradient;

  /// Selection of the gradient component if _evaluate_gradient is true; otherwise ignored
  unsigned int _gradient_component;

  /// Pointer to SolutionUserObject containing the solution of interest
  const SolutionUserObjectBase * _solution_object_ptr;
};
