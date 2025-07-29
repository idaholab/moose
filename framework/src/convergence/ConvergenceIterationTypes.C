//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvergenceIterationTypes.h"
#include "MooseUtils.h"

namespace moose
{
namespace internal
{

ConvergenceIterationTypeRegistry &
ConvergenceIterationTypeRegistry::getRegistry()
{
  static ConvergenceIterationTypeRegistry conv_iter_type_registry;
  return conv_iter_type_registry;
}

const Convergence::IterationType &
ConvergenceIterationTypeRegistry::registerType(const std::string & name)
{
  const auto name_upper = MooseUtils::toUpper(name);

  const auto type_iter = _types.find(name_upper);
  if (type_iter != _types.items().end())
  {
    // Assume that this is not a duplicate name, but a "double registration" due
    // to the dynamic load functionality.
    return *type_iter;
  }
  else
    return _types.addConvergenceIterationType(name_upper);
}

} // internal
} // moose

namespace ConvergenceIterationTypes
{
const Convergence::IterationType &
registerType(const std::string & it_type)
{
  return moose::internal::ConvergenceIterationTypeRegistry::getRegistry().registerType(it_type);
}
}
