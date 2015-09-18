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
    _input_parameters(libMesh::n_threads())
{
}

InputParameterWarehouse::~InputParameterWarehouse()
{
}

InputParameters &
InputParameterWarehouse::addInputParameters(const std::string & name, InputParameters parameters, THREAD_ID tid /* =0 */)
{

  // Create the storage object
  std::vector<MooseObjectName> object_names;

  // Create the actual InputParameters object
  MooseSharedPointer<InputParameters> ptr(new InputParameters(parameters));

  // The object name, w/o the object base prefix (this is returned by MooseObject::name())
  ptr->addParam<std::string>("name", name, "The name of the object");

  // Build the vector of names this object shall be stored as
  object_names.push_back(MooseObjectName(ptr->get<std::string>("_moose_base"), name));

  // Input file syntax, if it exists (populated by MooseObjectAction)
  if (ptr->isParamValid("action_tag"))
    object_names.push_back(MooseObjectName(ptr->get<std::string>("action_tag"), name));

  // User-defined tag, if it exists (optional)
  if (ptr->isParamValid("control_tag"))
    object_names.push_back(MooseObjectName(ptr->get<std::string>("control_tag"), name));

  // Set the name and tid parameters
  ptr->addPrivateParam<THREAD_ID>("_tid", tid);
  ptr->allowCopy(false);  // no more copies allowed

  // Store the object in the warehouse
  for (std::vector<MooseObjectName>::const_iterator it = object_names.begin(); it != object_names.end(); ++it)
  {
    // Check that the Parameters do not already exist
    if (_input_parameters[tid].find(*it) != _input_parameters[tid].end())
      mooseError("A " << it->tag << " object already exists with the name " << name << ".\n");

    // Place the SharedPointer in the storage map
    _input_parameters[tid][*it] = ptr;
  }

  // Return a reference to the InputParameters object
  return *ptr;
}


InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & tag, const std::string & name, THREAD_ID tid /* =0 */)
{
  // Create object name
  MooseObjectName object_name(tag, name);

  // Located the object
  std::map<MooseObjectName, MooseSharedPointer<InputParameters> >::iterator iter = _input_parameters[tid].find(object_name);

  // Error if not found
  if (iter == _input_parameters[tid].end())
    mooseError("Unknown InputParameters object " << long_name);

  // Return a writable reference to the InputParameters object
  return iter->second.get();
}


const std::vector<InputParameters *> &
InputParameterWarehouse::all() const
{
  mooseError("The all() method is not active for InputParameterWarehouse");
  return _all_objects;
}
