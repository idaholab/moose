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

#ifndef REPORTER_H
#define REPORTER_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class Reporter;

template<>
InputParameters validParams<Reporter>();

/**
 * A class for storing data, it allows the user to change the value of the
 * postprocessor by altering the _my_value reference
 */
class Reporter : public GeneralPostprocessor
{
public:
  /**
   * Class constructor
   * @param name The name of the Reporter postprocessor
   * @param parameters The input parameters
   */
  Reporter(const std::string & name, InputParameters parameters);

  /**
   * No action taken
   */
  virtual void initialize() {}

  /**
   * No action taken
   */
  virtual void execute() {}

  /**
   * Set the _my_value to the supplied default value
   */
  virtual void initialSetup();

  /**
   * Returns the value stored in _my_value
   * @return The value of the postprocessor
   */
  virtual Real getValue();

protected:

  /// Reference to the value being stored in the associated PostprocessorData class
  PostprocessorValue & _my_value;

  /// Whether or not the value should be the global sum
  bool _sum;
};

#endif //REPORTER_H
