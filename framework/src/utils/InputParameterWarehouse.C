//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "InputParameterWarehouse.h"
#include "InputParameters.h"

InputParameterWarehouse::InputParameterWarehouse()
  : _input_parameters(libMesh::n_threads()), _controllable_items(libMesh::n_threads())
{
}

InputParameters &
InputParameterWarehouse::addInputParameters(const std::string & name,
                                            InputParameters & parameters,
                                            THREAD_ID tid /* =0 */)
{
  // Error if the name contains "::"
  if (name.find("::") != std::string::npos)
    mooseError("The object name may not contain '::' in the name: ", name);

  // Create the actual InputParameters object that will be reference by the objects
  std::shared_ptr<InputParameters> ptr = std::make_shared<InputParameters>(parameters);

  auto base = ptr->get<std::string>("_moose_base");

  // The object name defined by the base class name, this method of storing is used for
  // determining the uniqueness of the name
  MooseObjectName unique_name(base, name, "::");

  // Check that the Parameters do not already exist. We allow duplicate unique_names for
  // MooseVariableBase objects because we require duplication of the variable for reference and
  // displaced problems. We must also have std::pair(reference_var, reference_params) AND
  // std::pair(displaced_var, displaced_params) elements because the two vars will have different
  // values for _sys. It's a good thing we are using a multi-map as our underlying storage
  if (_input_parameters[tid].find(unique_name) != _input_parameters[tid].end() &&
      base != "MooseVariableBase")
    mooseError("A '",
               unique_name.tag(),
               "' object already exists with the name '",
               unique_name.name(),
               "'.\n");

  // Store the parameters according to the base name
  _input_parameters[tid].insert(
      std::pair<MooseObjectName, std::shared_ptr<InputParameters>>(unique_name, ptr));

  // Build a list of object names
  std::vector<MooseObjectName> object_names;
  object_names.push_back(unique_name);

  // Store the object according to the control tags
  if (ptr->isParamValid("control_tags"))
  {
    const std::vector<std::string> & tags = ptr->get<std::vector<std::string>>("control_tags");
    for (const auto & tag : tags)
    {
      if (!tag.empty())
      {
        _input_parameters[tid].insert(std::pair<MooseObjectName, std::shared_ptr<InputParameters>>(
            MooseObjectName(tag, name), ptr));
        object_names.emplace_back(tag, name);
      }
    }
  }

  // Store controllable parameters using all possible names
  for (libMesh::Parameters::iterator map_iter = ptr->begin(); map_iter != ptr->end(); ++map_iter)
  {
    const std::string & name = map_iter->first;
    libMesh::Parameters::Value * value = map_iter->second;

    if (ptr->isControllable(name))
      for (const auto & object_name : object_names)
      {
        MooseObjectParameterName param_name(object_name, name);
        _controllable_items[tid].emplace_back(std::make_shared<ControllableItem>(
            param_name, value, ptr->getControllableExecuteOnTypes(name)));
      }
  }

  // Set the name and tid parameters, and unique_name
  std::stringstream oss;
  oss << unique_name;

  ptr->addPrivateParam<std::string>("_unique_name", oss.str());
  ptr->addPrivateParam<std::string>("_object_name", name);
  ptr->addPrivateParam<THREAD_ID>("_tid", tid);
  ptr->allowCopy(false); // no more copies allowed

  // Return a reference to the InputParameters object
  return *ptr;
}

void
InputParameterWarehouse::removeInputParameters(const MooseObject & moose_object, THREAD_ID tid)
{
  auto moose_object_name_string = moose_object.parameters().get<std::string>("_unique_name");
  MooseObjectName moose_object_name(moose_object_name_string);
  _input_parameters[tid].erase(moose_object_name);
}

const InputParameters &
InputParameterWarehouse::getInputParametersObject(const std::string & name, THREAD_ID tid) const
{
  return getInputParameters(MooseObjectName(name), tid);
}

const InputParameters &
InputParameterWarehouse::getInputParametersObject(const std::string & tag,
                                                  const std::string & name,
                                                  THREAD_ID tid) const
{
  return getInputParameters(MooseObjectName(tag, name), tid);
}

const InputParameters &
InputParameterWarehouse::getInputParametersObject(const MooseObjectName & object_name,
                                                  THREAD_ID tid) const
{
  return getInputParameters(object_name, tid);
}

InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & name, THREAD_ID tid) const
{
  return getInputParameters(MooseObjectName(name), tid);
}

InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & tag,
                                            const std::string & name,
                                            THREAD_ID tid) const
{
  return getInputParameters(MooseObjectName(tag, name), tid);
}

InputParameters &
InputParameterWarehouse::getInputParameters(const MooseObjectName & object_name,
                                            THREAD_ID tid) const
{
  // Locate the InputParameters object and error if it was not located
  const auto iter = _input_parameters[tid].find(object_name);
  if (iter == _input_parameters[tid].end())
    mooseError("Unknown InputParameters object ", object_name);

  // Return a reference to the parameter
  return *(iter->second.get());
}

const std::multimap<MooseObjectName, std::shared_ptr<InputParameters>> &
InputParameterWarehouse::getInputParameters(THREAD_ID tid) const
{
  return _input_parameters[tid];
}

void
InputParameterWarehouse::addControllableParameterConnection(
    const MooseObjectParameterName & primary,
    const MooseObjectParameterName & secondary,
    bool error_on_empty /*=true*/)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::vector<ControllableItem *> primarys = getControllableItems(primary, tid);
    if (primarys.empty() && error_on_empty && tid == 0) // some objects only exist on tid 0
      mooseError("Unable to locate primary parameter with name ", primary);
    else if (primarys.empty())
      return;

    std::vector<ControllableItem *> secondarys = getControllableItems(secondary, tid);
    if (secondarys.empty() && error_on_empty && tid == 0) // some objects only exist on tid 0
      mooseError("Unable to locate secondary parameter with name ", secondary);
    else if (secondarys.empty())
      return;

    for (auto primary_ptr : primarys)
      for (auto secondary_ptr : secondarys)
        if (primary_ptr != secondary_ptr)
          primary_ptr->connect(secondary_ptr);
  }
}

void
InputParameterWarehouse::addControllableObjectAlias(const MooseObjectName & alias,
                                                    const MooseObjectName & secondary)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::vector<ControllableItem *> secondarys =
        getControllableItems(MooseObjectParameterName(secondary, "*"), tid);
    for (auto secondary_ptr : secondarys)
    {
      MooseObjectParameterName alias_param(alias, secondary_ptr->name().parameter());
      MooseObjectParameterName secondary_param(secondary, secondary_ptr->name().parameter());
      addControllableParameterAlias(alias_param, secondary_param);
    }
  }
}

void
InputParameterWarehouse::addControllableParameterAlias(const MooseObjectParameterName & alias,
                                                       const MooseObjectParameterName & secondary)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::vector<ControllableItem *> secondarys = getControllableItems(secondary, tid);
    if (secondarys.empty() && tid == 0) // some objects only exist on tid 0
      mooseError("Unable to locate secondary parameter with name ", secondary);

    for (auto secondary_ptr : secondarys)
      _controllable_items[tid].emplace_back(
          libmesh_make_unique<ControllableAlias>(alias, secondary_ptr));
  }
}

std::vector<ControllableItem *>
InputParameterWarehouse::getControllableItems(const MooseObjectParameterName & input,
                                              THREAD_ID tid /*=0*/) const
{
  std::vector<ControllableItem *> output;
  for (auto & ptr : _controllable_items[tid])
    if (ptr->name() == input)
      output.push_back(ptr.get());
  return output;
}

ControllableParameter
InputParameterWarehouse::getControllableParameter(const MooseObjectParameterName & input) const
{
  ControllableParameter cparam;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    for (auto it = _controllable_items[tid].begin(); it != _controllable_items[tid].end(); ++it)
      if ((*it)->name() == input)
        cparam.add(it->get());
  return cparam;
}

std::string
InputParameterWarehouse::dumpChangedControls(bool reset_changed) const
{
  std::stringstream oss;
  oss << std::left;

  for (const auto & item : _controllable_items[0])
    if (item->isChanged())
    {
      oss << item->dump(4);
      if (reset_changed)
        item->resetChanged();
    }
  return oss.str();
}
