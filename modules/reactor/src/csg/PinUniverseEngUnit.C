//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PinUniverseEngUnit.h"
#include "CSGZCylinder.h"
#include "RGMBEngUnitUtils.h"
#include "CSGUtils.h"

namespace CSG
{

PinUniverseEngUnit::PinUniverseEngUnit(const std::string & name,
                                       const std::vector<Real> & ring_radii,
                                       const std::vector<std::string> & fill_mats)
  : CSGUniverseEngUnit(name), _ring_radii(ring_radii), _fill_mats(fill_mats)
{
  unsigned int n_radial = _ring_radii.size();

  if (n_radial == 0)
    mooseError("Pin universe engineering unit must have at least one ring radius defined");

  // Check radii in ascending order
  if (n_radial > 1)
    for (const auto i : make_range(_ring_radii.size() - 1))
      if (_ring_radii[i] >= _ring_radii[i + 1])
        mooseError("Pin engineering unit must have ring radii defined in strictly ascending order");

  if (_fill_mats.size() != n_radial + 1)
    mooseError("Size of region IDs must be one more than the number of radial rings in pin");
}

std::unordered_map<std::string, AttributeVariant>
PinUniverseEngUnit::getAttributes() const
{
  return {{"ring_radii", _ring_radii}, {"fill_mats", _fill_mats}};
}

std::unique_ptr<CSGUniverseEngUnit>
PinUniverseEngUnit::clone() const
{
  return std::make_unique<PinUniverseEngUnit>(_name, _ring_radii, _fill_mats);
}

void
PinUniverseEngUnit::expandUnit()
{
  CSG::CSGRegion inner_region, outer_region, cell_region;

  // Add surfaces and regions for each pin ring
  // Iterate through all rings and define all corresponding surfaces, regions, and cells
  for (const auto i : index_range(_fill_mats))
  {
    const bool build_ring_surf = i < _ring_radii.size();
    if (build_ring_surf)
    {
      // Add surfaces corresponding to pin rings
      const auto & radius = _ring_radii[i];
      const auto surf_name = _name + "_radial_ring_" + std::to_string(i);
      std::unique_ptr<CSG::CSGSurface> ring_surf_ptr =
          std::make_unique<CSG::CSGZCylinder>(surf_name, 0, 0, radius);
      const auto & ring_surf = _internal_base->addSurface(std::move(ring_surf_ptr));

      // Define radial region for each incremental radial ring
      if (inner_region.getRegionType() == CSG::CSGRegion::RegionType::EMPTY)
      {
        // We are in the innermost radial region, the radial region is inner_region
        inner_region = CSGUtils::getInnerRegion({ring_surf}, Point(0, 0, 0));
        cell_region = inner_region;
      }
      else
      {
        // For all other regions, the radial region is the intersection of inner_region and
        // outer_region
        outer_region = ~inner_region;
        inner_region = CSGUtils::getInnerRegion({ring_surf}, Point(0, 0, 0));
        cell_region = inner_region & outer_region;
      }
    }
    else
      cell_region = ~inner_region;

    // Define all cells within pin domain
    auto cell_name = _name + "_cell_radial_" + std::to_string(i);
    const auto fill_mat = _fill_mats[i];
    _internal_base->createCell(cell_name, fill_mat, cell_region);
  }
}

} // namespace CSG
