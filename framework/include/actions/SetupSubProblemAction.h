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

#ifndef SETUPSUBPROBLEMACTION_H
#define SETUPSUBPROBLEMACTION_H

#include "Action.h"

class SetupSubProblemAction;

template<>
InputParameters validParams<SetupSubProblemAction>();

/**
 * Setup the SubProblem-derived classes
 */
class SetupSubProblemAction : public Action
{
public:
  SetupSubProblemAction(const std::string & name, InputParameters parameters);
  virtual ~SetupSubProblemAction();

  virtual void act();

protected:
  std::string _coord_sys;
  bool _fe_cache;
};


#endif /* SETUPSUBPROBLEMACTION_H */
