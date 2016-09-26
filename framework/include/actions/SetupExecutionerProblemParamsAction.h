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

#ifndef SETUPEXECUTIONERPROBLEMPARAMSACTION_H
#define SETUPEXECUTIONERPROBLEMPARAMSACTION_H

#include "Action.h"

class SetupExecutionerProblemParamsAction;

template<>
InputParameters validParams<SetupExecutionerProblemParamsAction>();

/// This action allows for solver parameters that fit well in the input file's
/// Executioner block to live inside the solver where they are actually set and
/// used.
class SetupExecutionerProblemParamsAction : public Action
{
public:
  SetupExecutionerProblemParamsAction(InputParameters params);

  virtual void act() override;
};

#endif // SETUPEXECUTIONERPROBLEMPARAMSACTION_H

