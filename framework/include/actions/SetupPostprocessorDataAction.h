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

#ifndef SETUPPOSTPROCESSORDATAACTION_H
#define SETUPPOSTPROCESSORDATAACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class SetupPostprocessorDataAction;

template <>
InputParameters validParams<SetupPostprocessorDataAction>();

/**
 * The PostprocessorInterface::hasPostprocessor method utilizes the PostprocessorData
 * to determine if a postprocessor exists. Since many objects (e.g., Functions) are created
 * prior to the creation of Postprocessors, the PostprocessorData must be populated much eariler
 * than the actual creation of the Postprocessors if the correct behavior of hasPostprocessor is
 * to be achieved. Hence, this action simply initializes the PostprocessorData.
 */
class SetupPostprocessorDataAction : public Action
{
public:
  /**
   * Class constructor
   * @param params Input parameters for the action
   */
  SetupPostprocessorDataAction(InputParameters params);

  virtual void act() override;
};

#endif // SETUPPOSTPROCESSORDATAACTION_H
