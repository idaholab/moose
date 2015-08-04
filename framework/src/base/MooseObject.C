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

#include "MooseObject.h"
#include "MooseApp.h"
#include "MooseUtils.h"

template<>
InputParameters validParams<MooseObject>()
{
  InputParameters params;
  return params;
}

MooseObject::MooseObject(const InputParameters & parameters) :
  ConsoleStreamInterface(*parameters.get<MooseApp *>("_moose_app")), // Can't call getParam before pars is set
  ParallelObject(*parameters.get<MooseApp *>("_moose_app")), // Can't call getParam before pars is set
  _app(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
  _pars(parameters),
  _name(getParam<std::string>("name")),
  _short_name(MooseUtils::shortName(_name))
{
}
