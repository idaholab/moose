//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADMaterial.h"

namespace Moose::Kokkos
{

InputParameters
ADMaterial::validParams()
{
  InputParameters params = Material::validParams();
  return params;
}

ADMaterial::ADMaterial(const InputParameters & parameters) : Material(parameters) {}

std::set<const ADMaterial *>
ADMaterial::adPropertyDeclarers() const
{
  std::set<const ADMaterial *> declarers;

  for (const auto & derivatives : _ad_read_derivatives)
    for (const auto * declarer : derivatives.declarers())
      if (const auto * material = dynamic_cast<const ADMaterial *>(declarer))
        declarers.insert(material);

  return declarers;
}

void
ADMaterial::resolveADSlots()
{
  if (_ad_slots_resolved)
    return;

  if (_ad_slots_resolving)
    mooseError("The Kokkos AD materials reading each other's AD properties form a cycle.");

  _ad_slots_resolving = true;

  // The input tables of the properties this material reads are those of their declarers, which
  // are resolved first
  for (const auto * declarer : adPropertyDeclarers())
    const_cast<ADMaterial *>(declarer)->resolveADSlots();

  const unsigned int max_slots = DispatcherRegistry::getProperty("max_ad_slots", type());

  _ad_setup.assign(*this, max_slots, _configuration.displacements(), kokkosMesh());
  _ad_slots = _ad_setup.table();

  const unsigned int displacement_slots = _ad_slots.hasDisplacements() ? AD_DISPLACEMENT_SLOTS : 0;

  for (auto & declaration : _ad_properties)
  {
    auto & map = declaration.map;
    std::vector<ADSlotBlock> stored;
    unsigned int width = 0;

    for (const auto s : make_range(map.size()))
      map[s] = libMesh::invalid_uint;

    if (declaration.inputs.empty())
    {
      // Every slot is stored at its own index
      width = max_slots + displacement_slots;

      for (const auto s : make_range(width))
        map[s] = s;

      stored = _ad_slots.hostBlocks();
    }
    else
    {
      // The listed inputs are stored consecutively in the order of the list, and the displacement
      // slots follow them. An accessor that carries no derivatives keeps its storage, which stays
      // zero. A listed property stands for the slot blocks of this material that its inputs map
      // to on any subdomain. Each slot block is stored once.
      std::vector<std::pair<const ADSlotBlock *, unsigned int>> listed;

      auto add_block = [&](const unsigned int first, const bool gradient)
      {
        const auto n = gradient ? AD_GRADIENT_SLOTS : AD_VALUE_SLOTS;

        if (first == libMesh::invalid_uint)
        {
          width += n;
          return;
        }

        for (const auto & block : _ad_slots.hostBlocks())
          if (block.first == first && block.gradient == gradient)
          {
            for (const auto & [other, _] : listed)
              if (other == &block)
                return;

            listed.emplace_back(&block, width);
          }

        for (const auto k : make_range(n))
          map[first + k] = width + k;

        width += n;
      };

      for (const auto & input : declaration.inputs)
        if (const auto * slot = input.slot())
          add_block(slot->first, input.gradient());
        else
        {
          const auto & property_map = input.property()->slotMap();
          const auto & mesh = kokkosMesh();

          for (const auto & [subdomain, inputs] : input.property()->derivatives().adInputs())
          {
            const auto & slots = property_map.slots[mesh.getContiguousSubdomainID(subdomain)];

            for (const auto & block : inputs)
              if (block.displacement_component == libMesh::invalid_uint &&
                  slots[block.first] != libMesh::invalid_uint)
                add_block(slots[block.first], block.gradient);
          }
        }

      for (const auto & [block, index] : listed)
      {
        auto moved = *block;
        moved.first = index;
        stored.push_back(moved);
      }

      for (const auto s : make_range(max_slots, max_slots + displacement_slots))
        map[s] = width + s - max_slots;

      for (const auto & block : _ad_slots.hostBlocks())
        if (block.displacement_component != libMesh::invalid_uint)
        {
          auto moved = block;
          moved.first = width + block.first - max_slots;
          stored.push_back(moved);
        }

      width += displacement_slots;
    }

    map.copyToDevice();

    materialData().setKokkosADInputs(
        _declare_suffix.empty()
            ? declaration.derivatives_name
            : MooseUtils::join(
                  std::vector<std::string>({declaration.derivatives_name, _declare_suffix}), "_"),
        this,
        stored,
        width);
  }

  _ad_slots_resolving = false;
  _ad_slots_resolved = true;
}

} // namespace Moose::Kokkos
