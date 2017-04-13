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

// MOOSE includes
#include "InputParameterWarehouse.h"
#include "InputParameters.h"

InputParameterWarehouse::InputParameterWarehouse() : _input_parameters(libMesh::n_threads()) {}

InputParameters &
InputParameterWarehouse::addInputParameters(const std::string & name,
                                            InputParameters parameters,
                                            THREAD_ID tid /* =0 */)
{
  // Error if the name contains "::"
  if (name.find("::") != std::string::npos)
    mooseError("The object name may not contain '::' in the name: ", name);

  // Create the actual InputParameters object that will be reference by the objects
  std::shared_ptr<InputParameters> ptr = std::make_shared<InputParameters>(parameters);

  // Set the name parameter to the object being created
  ptr->set<std::string>("_object_name") = name;

  // The object name defined by the base class name, this method of storing is used for
  // determining the uniqueness of the name
  MooseObjectName unique_name(ptr->get<std::string>("_moose_base"), name, "::");

  // Check that the Parameters do not already exist
  if (_input_parameters[tid].find(unique_name) != _input_parameters[tid].end())
    mooseError("A '",
               unique_name.tag(),
               "' object already exists with the name '",
               unique_name.name(),
               "'.\n");

  // Store the parameters according to the base name
  _input_parameters[tid].insert(
      std::pair<MooseObjectName, std::shared_ptr<InputParameters>>(unique_name, ptr));

  // Store the object according to the control tags
  if (ptr->isParamValid("control_tags"))
  {
    const std::vector<std::string> & tags = ptr->get<std::vector<std::string>>("control_tags");
    for (const auto & tag : tags)
      _input_parameters[tid].insert(std::pair<MooseObjectName, std::shared_ptr<InputParameters>>(
          MooseObjectName(tag, name), ptr));
  }

  // Set the name and tid parameters
  ptr->addPrivateParam<THREAD_ID>("_tid", tid);
  ptr->allowCopy(false); // no more copies allowed

  // Return a reference to the InputParameters object
  return *ptr;
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
InputParameterWarehouse::addControllableParameterConnection(const MooseObjectParameterName & master,
                                                            const MooseObjectParameterName & slave)
{
  _input_parameter_links[master].push_back(slave);
}
