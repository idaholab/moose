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

#ifndef PERFLOGOUTPUTACTION_H
#define PERFLOGOUTPUTACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class PerfLogOutputAction;

template<>
InputParameters validParams<PerfLogOutputAction>();

/**
 * In general the Console output object controls all aspects of toggling performance logging.
 * However, there are two scenarios where additional control is needed:
 *   (1) If there is no Console object the logging must be disabled
 *   (2) If --timing is used from the command-line all logs are enabled, overriding whatever
 *       the Console object(s) are doing
 */
class PerfLogOutputAction : public Action
{
public:

  /**
   * Class constructor
   * @param name The action name
   * @param params The action parameters
   */
  PerfLogOutputAction(const std::string & name, InputParameters params);

  /**
   * Class destructor
   */
  virtual ~PerfLogOutputAction();

  /**
   * Set the appropriate state for the performance logs given the outputs and use of --timing
   */
  virtual void act();
};

#endif //PERFLOGOUTPUTACTION_H
