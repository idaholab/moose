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

#include "AppFactory.h"
#include "CommandLine.h"

AppFactory * AppFactory::_instance = NULL;

AppFactory *AppFactory::instance()
{
  if (!_instance)
    _instance = new AppFactory;

  return _instance;
}

AppFactory::~AppFactory()
{
}

MooseApp *
AppFactory::createApp(std::string app_type, int argc, char ** argv)
{
  CommandLine command_line(argc, argv);

  InputParameters app_params = AppFactory::instance()->getValidParams(app_type);

  command_line.addCommandLineOptionsFromParams(app_params);
  command_line.populateInputParams(app_params);

  app_params.set<int>("_argc") = argc;
  app_params.set<char**>("_argv") = argv;

  return AppFactory::instance()->create(app_type, "main", app_params);
}

void
AppFactory::release()
{
  delete _instance;
}

InputParameters
AppFactory::getValidParams(const std::string & name)
{
  if (_name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + name + "' is not a registered object\n\n");

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

MooseApp *
AppFactory::create(const std::string & obj_name, const std::string & name, InputParameters parameters)
{
  if (_name_to_build_pointer.find(obj_name) == _name_to_build_pointer.end())
    mooseError("Object '" + obj_name + "' was not registered.");

  // Check to make sure that all required parameters are supplied
  parameters.checkParams("");

  return (*_name_to_build_pointer[obj_name])(name, parameters);
}
