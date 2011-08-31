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

#ifndef SETUPDEBUGACTION_H_
#define SETUPDEBUGACTION_H_

#include "Action.h"

class SetupDebugAction;

template<>
InputParameters validParams<SetupDebugAction>();

/**
 *
 */
class SetupDebugAction : public Action
{
public:
  SetupDebugAction(const std::string & name, InputParameters parameters);
  virtual ~SetupDebugAction();

  virtual void act();

protected:
  unsigned int _top_residuals;
  bool _show_actions;
};


#endif /* SETUPDEBUGACTION_H_ */
