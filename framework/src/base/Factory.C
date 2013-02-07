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

#include "Factory.h"
#include "MooseApp.h"

Factory::Factory(MooseApp & app):
    _app(app)
{
}



Factory::~Factory()
{
}

InputParameters
Factory::getValidParams(const std::string & name)
{
  if (_name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + name + "' is not a registered object\n\n");

  InputParameters params = _name_to_params_pointer[name]();
  params.addPrivateParam("_moose_app", &_app);
  return params;
}

MooseObject *
Factory::create(const std::string & obj_name, const std::string & name, InputParameters parameters)
{
  if (_name_to_build_pointer.find(obj_name) == _name_to_build_pointer.end())
    mooseError("Object '" + obj_name + "' was not registered.");

  // Check to make sure that all required parameters are supplied
  parameters.checkParams(name);

  return (*_name_to_build_pointer[obj_name])(name, parameters);
}
