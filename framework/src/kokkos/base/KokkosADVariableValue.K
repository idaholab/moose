//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// KokkosKernels must see its headers before the MOOSE Kokkos macros, as in KokkosArray.K, because
// unity builds compile this file ahead of it
#include "KokkosBlas.hpp"

#include "KokkosADVariableValue.h"

namespace Moose::Kokkos
{

unsigned int
ADSlotTable::add(const unsigned int sys, const unsigned int var, const bool gradient)
{
  // A quantity read by several accessors shares one slot block
  for (const auto & block : _host_blocks)
    if (block.sys == sys && block.var == var && block.gradient == gradient)
      return block.first;

  const auto first = _num_slots;
  _host_blocks.push_back({sys, var, first, gradient});
  _num_slots += gradient ? AD_GRADIENT_SLOTS : AD_VALUE_SLOTS;

  return first;
}

void
ADSlotTable::addDisplacements(const std::vector<std::pair<unsigned int, unsigned int>> & components,
                              const std::vector<bool> & seeded)
{
  for (unsigned int c = 0; c < 3; ++c)
    _displacement_first[c] = libMesh::invalid_uint;

  for (const auto c : make_range(static_cast<unsigned int>(components.size())))
  {
    const auto first = _num_slots + c * (AD_VALUE_SLOTS + AD_GRADIENT_SLOTS);
    const auto [sys, var] = components[c];

    if (seeded[c])
    {
      _displacement_first[c] = first;
      _host_blocks.push_back({sys, var, first, false, c});
      _host_blocks.push_back({sys, var, first + AD_VALUE_SLOTS, true, c});
    }
  }

  _num_slots += AD_DISPLACEMENT_SLOTS;
  _has_displacements = true;
}

void
ADSlotTable::finalize()
{
  _num_blocks = _host_blocks.size();
  _blocks = _host_blocks;
}

} // namespace Moose::Kokkos
