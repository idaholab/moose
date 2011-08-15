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

#ifndef SETUPOVERSAMPLINGACTION_H
#define SETUPOVERSAMPLINGACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "SetupOutputAction.h"

#include <string>

class SetupOverSamplingAction;
class Output;

template<>
InputParameters validParams<SetupOverSamplingAction>();


class SetupOverSamplingAction : public SetupOutputAction
{
public:
  SetupOverSamplingAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // SETUPOVERSAMPLINGACTION_H
