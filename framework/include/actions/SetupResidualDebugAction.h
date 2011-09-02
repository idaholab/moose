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

#ifndef SETUPRESIDUALDEBUGACTION_H
#define SETUPRESIDUALDEBUGACTION_H

#include "Action.h"

class SetupResidualDebugAction;

template<>
InputParameters validParams<SetupResidualDebugAction>();

/**
 *
 */
class SetupResidualDebugAction : public Action
{
public:
  SetupResidualDebugAction(const std::string & name, InputParameters parameters);
  virtual ~SetupResidualDebugAction();

  virtual void act();

protected:
  std::vector<std::string> _show_var_residual;
};


#endif /* SETUPRESIDUALDEBUGACTION_H */
