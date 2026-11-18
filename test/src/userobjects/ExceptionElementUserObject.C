//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExceptionElementUserObject.h"

#include <stdexcept>

registerMooseObject("MooseTestApp", ExceptionElementUserObject);

InputParameters
ExceptionElementUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Test element user object that throws a std::runtime_error from execute on a single rank");
  params.addRequiredParam<processor_id_type>("rank", "The rank on which to throw from execute");
  return params;
}

ExceptionElementUserObject::ExceptionElementUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters), _rank(getParam<processor_id_type>("rank"))
{
}

void
ExceptionElementUserObject::execute()
{
  if (processor_id() == _rank)
    throw std::runtime_error("ExceptionElementUserObject intentional execute exception");
}

void
ExceptionElementUserObject::finalize()
{
  // Communicate like the finalize() of a typical element user object, so that a rank skipping the
  // exception synchronization cannot pass through here alone
  _communicator.barrier();
}
