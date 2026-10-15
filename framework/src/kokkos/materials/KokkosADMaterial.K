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

void
ADMaterial::initialSetup()
{
  Material::initialSetup();

  _ad_setup.assign(*this,
                   DispatcherRegistry::getProperty("max_ad_slots", type()),
                   _configuration.displacements());
  _ad_slots = _ad_setup.table();

  const unsigned int max_slots = DispatcherRegistry::getProperty("max_ad_slots", type());

  for (auto & declaration : _ad_properties)
  {
    auto & map = declaration.map;
    std::vector<ADSlotBlock> stored;

    if (declaration.inputs.empty())
    {
      // Every slot is stored at its own index
      for (const auto s : make_range(map.size()))
        map[s] = s;

      stored = _ad_slots.hostBlocks();
    }
    else
    {
      for (const auto s : make_range(map.size()))
        map[s] = libMesh::invalid_uint;

      // The listed inputs are stored consecutively in the order of the list, and the displacement
      // slots follow them. Inputs that carry no derivatives keep their storage, which stays zero.
      unsigned int index = 0;
      for (const auto & input : declaration.inputs)
      {
        const auto first = input.slot().first;

        if (first != libMesh::invalid_uint)
        {
          for (const auto k : make_range(input.numSlots()))
            map[first + k] = index + k;

          for (const auto & block : _ad_slots.hostBlocks())
            if (block.first == first && block.gradient == input.gradient())
            {
              auto moved = block;
              moved.first = index;
              stored.push_back(moved);
            }
        }

        index += input.numSlots();
      }

      for (const auto s : make_range(max_slots, map.size()))
        map[s] = index + s - max_slots;

      for (const auto & block : _ad_slots.hostBlocks())
        if (block.displacement_component != libMesh::invalid_uint)
        {
          auto moved = block;
          moved.first = index + block.first - max_slots;
          stored.push_back(moved);
        }
    }

    map.copyToDevice();

    materialData().setKokkosADInputs(
        _declare_suffix.empty()
            ? declaration.derivatives_name
            : MooseUtils::join(
                  std::vector<std::string>({declaration.derivatives_name, _declare_suffix}), "_"),
        this,
        stored);
  }
}

} // namespace Moose::Kokkos
