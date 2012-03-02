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
  InputParameters params = validParams<ObjectAction>();
  params.addRequiredParam<std::string>("type", "A string representing the Moose Object that will be built by this Action");
  return params;
}

MooseObjectAction::MooseObjectAction(const std::string & name, InputParameters params) :
    ObjectAction(name, params),
    _type(getParam<std::string>("type")),

    // We will create a second parameters object from the main factory unless instructed otherwise
    _moose_object_pars(!params.have_parameter<bool>("skip_param_construction") ||
                       (params.have_parameter<bool>("skip_param_construction") &&
                        !params.get<bool>("skip_param_construction"))
                       ? Factory::instance()->getValidParams(_type) : validParams<MooseObject>())
{
}

void
MooseObjectAction::addParamsPtrs(std::vector<InputParameters *> & param_ptrs)
{
  Action::addParamsPtrs(param_ptrs);
  param_ptrs.push_back(&_moose_object_pars);
}
