//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"

#include "ParameterRegistry.h"

namespace Moose
{
ParameterRegistry &
ParameterRegistry::get()
{
  static ParameterRegistry * registry = nullptr;
  if (!registry)
    registry = new ParameterRegistry();
  return *registry;
}

void
ParameterRegistry::set(libMesh::Parameters::Value & value, const hit::Field & field) const
{
  const auto key = value.type();
  const auto it = _registry.find(key);
  if (it == _registry.end())
    mooseError("ParameterRegistry::set(): Parameter type '", key, "' is not registered");

  // Catch all mooseErrors so that they can be accumulated during
  // parsing and building instead of ending the run
  Moose::ScopedThrowOnError scoped_throw_on_error;

  it->second(value, field);
}
}
