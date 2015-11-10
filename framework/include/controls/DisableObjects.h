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

#ifndef DISABLEOBJECTS_H
#define DISABLEOBJECTS_H

// MOOSE includes
#include "Control.h"

// Forward declarations
class DisableObjects;
class Function;

template<>
InputParameters validParams<DisableObjects>();

/**
 * A basic control for disabling objects for a portion of the simulation.
 */
class DisableObjects : public Control
{
public:

  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  DisableObjects(const InputParameters & parameters);

  /**
   * Evaluate the function and set the parameter value
   */
  virtual void execute();

private:

  /// List of objects to disable
  const std::vector<std::string> & _disable;

  /// The time to begin disabling the supplied object tags (defaults to the simulation start time)
  Real _start_time;

  /// The time to stop disabling the supplied object tags (defaults to the end of the simulation)
  Real _end_time;
};

#endif // DISABLEOBJECTS_H
