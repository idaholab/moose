//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMaterialProperty.h"

namespace Moose
{
namespace Kokkos
{
std::unordered_map<PropertyKey, PropertyStore> MaterialPropertyBase::_store;
std::unordered_map<PropertyKey, PropertyLoad> MaterialPropertyBase::_load;
} // namespace Kokkos
} // namespace Moose
