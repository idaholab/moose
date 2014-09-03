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

#ifndef SETUPOUTPUTNAMESACTION_H
#define SETUPOUTPUTNAMESACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class SetupOutputNamesAction;

template<>
InputParameters validParams<SetupOutputNamesAction>();

/**
 * Populates the list of available output names in the OutputWarehouse.
 *
 * This allows for objects to reference output names prior to the output
 * object construction.
 */
class SetupOutputNamesAction : public Action
{
public:

  /**
   * Class constructor
   */
  SetupOutputNamesAction(const std::string & name, InputParameters params);

  /**
   * Class destructor
   */
  virtual ~SetupOutputNamesAction();

  /**
   * Populates the available output names in the OutputWarehouse
   */
  virtual void act();
};

#endif //SETUPOUTPUTNAMESACTION_H
