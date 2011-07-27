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

#ifndef ADDPERIODICBCACTION_H
#define ADDPERIODICBCACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

// libMesh includes
#include "periodic_boundaries.h"

class AddPeriodicBCAction;

template<>
InputParameters validParams<AddPeriodicBCAction>();

/**
 * This Action adds a periodic boundary to the problem. Note that Periodic Boundaries
 * are not MooseObjects so you need not specify a type for these boundaries.  If you
 * do, it will currently be ignored by this Action.
 */
class AddPeriodicBCAction : public Action
{
public:
  AddPeriodicBCAction(const std::string & name, InputParameters params);

  virtual void act();

protected:
  void setPeriodicVars(PeriodicBoundary & p, const std::vector<std::string> & var_names);
};

#endif // ADDPERIODICBCACTION_H
