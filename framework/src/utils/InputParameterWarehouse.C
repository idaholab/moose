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
    _syntax_to_index(libMesh::n_threads()),
    _system_to_index(libMesh::n_threads())
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

  // Set the system name, if it exists
  if (ptr->isParamValid("_moose_base"))
    container.system = ptr->get<std::string>("_moose_base");

  // Input file syntax, if it exists
  if (ptr->isParamValid("_syntax"))
    container.syntax = ptr->get<std::string>("_syntax");

  // Define the two naming conventions
  std::string syntax_name = container.syntax + "/" + container.object;
  std::string system_name = container.system + "::" + container.object;

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
  _syntax_to_index[tid].insert(std::pair<std::string, unsigned int>(syntax_name, index));
  _system_to_index[tid].insert(std::pair<std::string, unsigned int>(system_name, index));

  return *ptr;
}


InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & long_name, THREAD_ID tid /* =0 */)
{
  unsigned int index;
  std::map<std::string, unsigned int>::iterator system_iter, syntax_iter;

  system_iter = _system_to_index[tid].find(long_name);
  syntax_iter = _syntax_to_index[tid].find(long_name);
  if (system_iter != _system_to_index[tid].end())
    index = system_iter->second;

  else if (syntax_iter != _syntax_to_index[tid].end())
    index = syntax_iter->second;

  else
    mooseError("Unknown InputParameters object " << long_name);

  return *(_input_parameters[tid][index].parameters.get());
}


const std::vector<InputParameters *> &
InputParameterWarehouse::all() const
{
  mooseError("The all() method is not active for InputParameterWarehouse");
  return _all_objects;
}
