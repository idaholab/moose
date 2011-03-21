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

#include "MooseObjectAction.h"
#include "Factory.h"

template<>
InputParameters validParams<MooseObjectAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>("type", "A string representing the Moose Object that will be built by this Action");
  return params;
}

MooseObjectAction::MooseObjectAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _type(getParam<std::string>("type")),
    _moose_object_pars(Factory::instance()->getValidParams(_type))
{
}

void
MooseObjectAction::addParamsPtrs(std::vector<InputParameters *> & param_ptrs)
{
  Action::addParamsPtrs(param_ptrs);
  param_ptrs.push_back(&_moose_object_pars);
}
