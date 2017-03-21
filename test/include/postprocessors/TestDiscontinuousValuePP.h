/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TESTDISCONTINUOUSVALUEPP_H
#define TESTDISCONTINUOUSVALUEPP_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class TestDiscontinuousValuePP;
class SolutionUserObject;

template <>
InputParameters validParams<TestDiscontinuousValuePP>();

/**
 * Compute the value of a variable or the gradient at a specified location.
 * Takes multivalued functions at boundaries into account
 */
class TestDiscontinuousValuePP : public GeneralPostprocessor
{
public:
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

#endif /* TestDiscontinuousValuePP_H */
