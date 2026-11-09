//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceDomainUserObject.h"

class InterfaceDomainUserObjectCallCount : public InterfaceDomainUserObject
{
public:
  static InputParameters validParams();

  InterfaceDomainUserObjectCallCount(const InputParameters & parameters);

  void initialize() override;
  void executeOnBoundary() override;
  void executeOnInterface() override;
  void finalize() override;
  void threadJoin(const UserObject & y) override;

private:
  /// Whether to error if executeOnBoundary is called on an interface boundary
  const bool _check_boundary_calls;

  /// Whether to error if executeOnInterface calls do not match expected_interface_calls
  const bool _check_interface_calls;

  /// Count of calls to executeOnInterface
  unsigned int _num_interface_calls;

  /// Count of calls to executeOnBoundary on interface boundaries (should stay 0)
  unsigned int _num_boundary_calls_on_interface;
};
