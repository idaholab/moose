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
  std::vector<InputParameters *>::iterator it;
  for (it = _all_objects.begin(); it != _all_objects.end(); ++it)
    delete *it;
}

const InputParameters &
InputParameterWarehouse::addInputParameters(const InputParameters & params)
{

  InputParameters * ptr = new InputParameters(params);


  std::string name = ptr->get<std::string>("long_name");
  _all_objects.push_back(ptr);
  _name_to_pointer[name] = ptr;

//aparams->print();

  return *_name_to_pointer[name];
}

const InputParameters &
InputParameterWarehouse::getInputParameters(const std::string & name)
{
  return *_name_to_pointer[name];
}
