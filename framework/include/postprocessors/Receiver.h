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

#ifndef RECEIVER_H
#define RECEIVER_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class Receiver;

template<>
InputParameters validParams<Receiver>();

/**
 * A class for storing data, it allows the user to change the value of the
 * postprocessor by altering the _my_value reference
 */
class Receiver : public GeneralPostprocessor
{
public:
  /**
   * Class constructor
   * @param name The name of the Receiver postprocessor
   * @param parameters The input parameters
   */
  Receiver(const std::string & name, InputParameters parameters);

  /**
   * No action taken
   */
  virtual void initialize() {}

  /**
   * No action taken
   */
  virtual void execute() {}

  /**
   * Returns the value stored in _my_value
   * @return A const reference to the value of the postprocessor
   */
  virtual Real getValue();

  /**
   * Initial setup function for applying the default value
   */
  virtual void initialSetup();

protected:

  /// Reference to the value being stored in the associated PostprocessorData class
  const PostprocessorValue & _my_value;

};

#endif //RECEIVER_H
