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
class SolutionUserObject;

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

  virtual void initialize(){};

  virtual void execute(){};

  virtual void finalize(){};

  /// initialSetup gets the pointer to the solution UO
  virtual void initialSetup();

  virtual Real getValue();

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
  const SolutionUserObject * _solution_object_ptr;
};
