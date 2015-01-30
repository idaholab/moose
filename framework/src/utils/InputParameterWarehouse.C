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
    Warehouse<InputParameters>()
{
}

InputParameterWarehouse::~InputParameterWarehouse()
{
}

const InputParameters &
InputParameterWarehouse::addInputParameters(const InputParameters & parameters)
{
  // Create the actual InputParameters object to store and reference from the objects
  MooseSharedPointer<InputParameters> ptr( new InputParameters(parameters));

  // Store the object in the warehouse
  std::string name = ptr->get<std::string>("_name");
   _name_to_shared_pointer[name] = ptr;

  // Return a const reference, this just saves calling the get method in Factory::create
  return *_name_to_shared_pointer[name];
}

const InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & name)
{
  return *_name_to_shared_pointer[name];
}

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

const std::vector<InputParameters *> &
InputParameterWarehouse::all()
{
  mooseError("The all() method is not active for InputParameterWarehouse");
  return _all_objects;
}
