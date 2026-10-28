//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceDomainUserObjectCallCount.h"

registerMooseObject("MooseTestApp", InterfaceDomainUserObjectCallCount);

InputParameters
InterfaceDomainUserObjectCallCount::validParams()
{
  InputParameters params = InterfaceDomainUserObject::validParams();
  params.addParam<unsigned int>(
      "expected_interface_calls",
      "If provided, verifies that executeOnInterface is called exactly this number of times.");
  params.addParam<bool>(
      "check_boundary_calls",
      true,
      "Whether to error if executeOnBoundary is called on an interface boundary.");
  params.addParam<bool>(
      "check_interface_calls",
      true,
      "Whether to error if executeOnInterface calls do not match expected_interface_calls.");
  return params;
}

InterfaceDomainUserObjectCallCount::InterfaceDomainUserObjectCallCount(
    const InputParameters & parameters)
  : InterfaceDomainUserObject(parameters),
    _check_boundary_calls(getParam<bool>("check_boundary_calls")),
    _check_interface_calls(getParam<bool>("check_interface_calls")),
    _num_interface_calls(0),
    _num_boundary_calls_on_interface(0)
{
}

void
InterfaceDomainUserObjectCallCount::initialize()
{
  InterfaceDomainUserObject::initialize();
  _num_interface_calls = 0;
  _num_boundary_calls_on_interface = 0;
}

void
InterfaceDomainUserObjectCallCount::executeOnBoundary()
{
  if (_interface_bnd_ids.count(_current_boundary_id))
    _num_boundary_calls_on_interface++;

  InterfaceDomainUserObject::executeOnBoundary();
}

void
InterfaceDomainUserObjectCallCount::executeOnInterface()
{
  if (!_interface_bnd_ids.count(_current_boundary_id))
    mooseError("In ",
               name(),
               ": executeOnInterface called with _current_boundary_id = ",
               _current_boundary_id,
               ", which is not in interface_boundaries");

  _num_interface_calls++;

  InterfaceDomainUserObject::executeOnInterface();
}

void
InterfaceDomainUserObjectCallCount::finalize()
{
  gatherSum(_num_interface_calls);
  gatherSum(_num_boundary_calls_on_interface);

  if (_check_boundary_calls && _num_boundary_calls_on_interface > 0)
    mooseError("In ",
               name(),
               ": executeOnBoundary was called on an interface boundary ",
               _num_boundary_calls_on_interface,
               " time(s)");

  if (_check_interface_calls && isParamValid("expected_interface_calls"))
  {
    const auto expected = getParam<unsigned int>("expected_interface_calls");
    if (_num_interface_calls != expected)
      mooseError("In ",
                 name(),
                 ": expected executeOnInterface to be called ",
                 expected,
                 " time(s), but it was called ",
                 _num_interface_calls,
                 " time(s)");
  }

  InterfaceDomainUserObject::finalize();
}

void
InterfaceDomainUserObjectCallCount::threadJoin(const UserObject & y)
{
  InterfaceDomainUserObject::threadJoin(y);
  const auto & uo = dynamic_cast<const InterfaceDomainUserObjectCallCount &>(y);
  _num_interface_calls += uo._num_interface_calls;
  _num_boundary_calls_on_interface += uo._num_boundary_calls_on_interface;
}
