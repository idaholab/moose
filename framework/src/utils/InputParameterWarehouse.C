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

#include "InputParameterWarehouse.h"
#include "InputParameters.h"

InputParameterWarehouse::InputParameterWarehouse() :
    Warehouse<InputParameters>(),
    _input_parameters(libMesh::n_threads()),
    _system_to_index(libMesh::n_threads()),
    _syntax_to_index(libMesh::n_threads()),
    _tag_to_index(libMesh::n_threads())
{
}

InputParameterWarehouse::~InputParameterWarehouse()
{
}

InputParameters &
InputParameterWarehouse::addInputParameters(std::string name, InputParameters parameters, THREAD_ID tid /* =0 */)
{

  // Create the storage object
  InputParametersContainer container;
  container.object = name;

  // Create the actual InputParameters object to store and reference from the objects
  MooseSharedPointer<InputParameters> ptr(new InputParameters(parameters));
  container.parameters = ptr;

  // The object name, w/o the object base prefix (this is returned by MooseObject::name())
  ptr->addParam<std::string>("name", name, "The name of the object");

  // Set the system name
  container.system = ptr->get<std::string>("_moose_base");
  std::string system_name = container.system + "::" + container.object;

  // Input file syntax, if it exists
  if (ptr->isParamValid("_syntax"))
    container.syntax = ptr->get<std::string>("_syntax");

  // User-defined tag, if it exists (optional)
  if (ptr->isParamValid("control_tag"))
    container.tag = ptr->get<std::string>("control_tag");

  // Check that the Parameters do not already exist
  if (_system_to_index[tid].find(system_name) != _system_to_index[tid].end())
    mooseError("A " << container.system << " object already exists with the name " << name << ".\n");

  // Set the name and tid parameters
  ptr->addPrivateParam<THREAD_ID>("_tid", tid);
  ptr->allowCopy(false);  // no more copies allowed

  // Store the object in the warehouse
  _input_parameters[tid].push_back(container);
  unsigned int index = _input_parameters.size() - 1;

  // Store the name for checking that names are unique and name-based lookup
  _system_to_index[tid].insert(std::pair<std::string, unsigned int>(system_name, index));

  if (ptr->isParamValid("_syntax"))
  {
    std::string syntax_name = container.syntax + "/" + container.object;
    _syntax_to_index[tid].insert(std::pair<std::string, unsigned int>(syntax_name, index));
  }

  if (ptr->isParamValid("control_tag"))
  {
    std::string tag_name = container.tag + "::" + container.object;
    _tag_to_index[tid].insert(std::pair<std::string, unsigned int>(tag_name, index));
  }

  return *ptr;
}


InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & long_name, THREAD_ID tid /* =0 */)
{
  // True when something is found
  bool found = false;

  // The location of the Inputparameters object
  unsigned int index;
  std::map<std::string, unsigned int>::iterator system_iter, syntax_iter, tag_iter;

  // Search for parameters using "system" naming convention
  system_iter = _system_to_index[tid].find(long_name);
  if (system_iter != _system_to_index[tid].end())
  {
    index = system_iter->second;
    found = true;
  }

  // Search for parameters using "syntax" naming convention
  if (!found)
  {
    syntax_iter = _syntax_to_index[tid].find(long_name);
    if (syntax_iter != _syntax_to_index[tid].end())
    {
      index = syntax_iter->second;
      found = true;
    }
  }

  // Search for parameters using "tag" naming convention
  if (!found)
  {
    tag_iter = _tag_to_index[tid].find(long_name);
    if (tag_iter != _tag_to_index[tid].end())
    {
      index = tag_iter->second;
      found = true;
    }
  }

  // Error if not found
  if (!found)
    mooseError("Unknown InputParameters object " << long_name);

  // Return a writable reference to the InputParameters object
  return *(_input_parameters[tid][index].parameters.get());
}


const std::vector<InputParameters *> &
InputParameterWarehouse::all() const
{
  mooseError("The all() method is not active for InputParameterWarehouse");
  return _all_objects;
}
