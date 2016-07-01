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

//Forward Declarations
class TestDiscontinuousValuePP;
class SolutionUserObject;

template<>
InputParameters validParams<TestDiscontinuousValuePP>();

/**
 * Compute the value of a variable or the gradient at a specified location.
 * Takes multivalued functions at boundaries into account
 */
class TestDiscontinuousValuePP : public GeneralPostprocessor
{
public:
  /**
   * Constructor.
   * @param parameters The input file parameters for this object
   */
  TestDiscontinuousValuePP(const InputParameters & parameters);

  /**
   * Destructor
   */
  virtual ~TestDiscontinuousValuePP(){};

  /// initialize override
  virtual void initialize() {};

  /// execute override
  virtual void execute() {};

  /// finalize override
  virtual void finalize() {};

  /// initialSetup gets the pointer to the solution UO
  virtual void initialSetup();

  /**
   * Returns the value of the variable at the specified location
   */
  virtual Real getValue();

protected:

  /// The variable name from which a values is to be extracted
  const VariableName & _variable_name;

  /// The point to locate, stored as a vector for use with reinitElemPhys
  Point _point;

  /// This option allows to switch between value and gradient evaluation
  bool _for_gradient;

  /// Selection of the gradient component if _for_gradient is true; otherwise ignored
  unsigned int _gradient_component;

  /// Pointer to SolutionUserObject containing the solution of interest
  const SolutionUserObject * _solution_object_ptr;
};

#endif /* TestDiscontinuousValuePP_H */
