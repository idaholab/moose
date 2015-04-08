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
    _name_to_shared_pointer(libMesh::n_threads())
{
}

InputParameterWarehouse::~InputParameterWarehouse()
{
}

InputParameters &
InputParameterWarehouse::addInputParameters(const std::string & name, InputParameters parameters, THREAD_ID tid /* =0 */)
{

  // Check that the Parameters do not already exist
  if (_name_to_shared_pointer[tid].find(name) != _name_to_shared_pointer[tid].end())
    mooseError("An object already exists with the name " << name);

  // Create the actual InputParameters object to store and reference from the objects
  MooseSharedPointer<InputParameters> ptr(new InputParameters(parameters));

  // Set the name and tid parameters
  ptr->addParam<std::string>("name", name, "The complete name of the object");
  ptr->addPrivateParam<THREAD_ID>("_tid", tid);

  // Store the object in the warehouse
  _name_to_shared_pointer[tid].insert(std::pair<std::string, MooseSharedPointer<InputParameters> >(name, ptr));

  return *_name_to_shared_pointer[tid][name];
}

const InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & name, THREAD_ID tid /* =0 */)
{
  return *_name_to_shared_pointer[tid][name];
}

/*
InputParameterIterator
InputParameterWarehouse::begin()
{
  return _name_to_shared_pointer.begin();
}

InputParameterIterator
InputParameterWarehouse::end()
{
  return _name_to_shared_pointer.end();
}
*/

const std::vector<InputParameters *> &
InputParameterWarehouse::all() const
{
  mooseError("The all() method is not active for InputParameterWarehouse");
  return _all_objects;
}
