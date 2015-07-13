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

#ifndef TIMEPOSTPROCESSOR_H
#define TIMEPOSTPROCESSOR_H

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward declerations
class TimePostprocessor;

template<>
InputParameters validParams<TimePostprocessor>();

/**
 * Reports the simulation time as a postprocessor.
 *
 * This is needed by the Exodus output object when using the "ensight_time" time flag.
 */
class TimePostprocessor : public GeneralPostprocessor
{
public:

  /**
   * Class constructor
   * @param name Name of the postprocessor
   * @param parameters InputParameters for the object being created
   */
  TimePostprocessor(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~TimePostprocessor(){};

  ///@}
  /**
   * These methods are intentionally empty
   */
  void execute(){};
  void initialize(){};
  void finalize(){};
  ///@}

  /**
   * Returns the current simulation time
   */
  virtual PostprocessorValue getValue();
};

#endif //TIMEPOSTPROCESSOR_H
