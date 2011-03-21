#include "Factory.h"


Factory *Factory::instance()
{
  static Factory *instance;
  if (!instance)
    instance = new Factory;

  return instance;
}

InputParameters
Factory::getValidParams(const std::string & name)
{
  if (_name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + name + "' is not a registered object\n\n");

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

Object *
Factory::create(const std::string & obj_name, const std::string & name, InputParameters parameters)
{
  if (_name_to_build_pointer.find(obj_name) != _name_to_build_pointer.end())
    return (*_name_to_build_pointer[obj_name])(name, parameters);
  else
    mooseError("Object '" + obj_name + "' was not registered.");
}
