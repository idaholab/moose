//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DuctedPinEngUnit.h"
#include "PinUniverseEngUnit.h"
#include "CSGNPolygonUnit.h"
#include "CSGUtils.h"
#include "CSGPlane.h"

namespace CSG
{

DuctedPinEngUnit::DuctedPinEngUnit(const std::string & name,
                                   const std::string & geometry_type,
                                   const std::vector<Real> & ring_radii,
                                   const std::vector<Real> & duct_apothems,
                                   const std::vector<std::vector<std::string>> & region_names,
                                   const std::vector<Real> & axial_plane_levels,
                                   const std::vector<std::string> & axial_plane_names)
  : CSGUniverseEngUnit(name),
    _ring_radii(ring_radii),
    _duct_apothems(duct_apothems),
    _region_names(region_names),
    _axial_plane_levels(axial_plane_levels),
    _axial_plane_names(axial_plane_names)
{
  // Check radii in ascending order
  if (_ring_radii.size() > 1)
    for (const auto i : make_range(_ring_radii.size() - 1))
      if (_ring_radii[i] >= _ring_radii[i + 1])
        mooseError(
            "Ducted pin engineering unit must have ring radii defined in strictly ascending order");

  // Check duct apothems in ascending order
  if (_duct_apothems.size() > 1)
    for (const auto i : make_range(_duct_apothems.size() - 1))
      if (_duct_apothems[i] >= _duct_apothems[i + 1])
        mooseError("Ducted pin engineering unit must have duct apothems defined in strictly "
                   "ascending order");

  // Check axial plane levels in ascending order
  if (_axial_plane_levels.size() > 1)
    for (const auto i : make_range(_axial_plane_levels.size() - 1))
      if (_axial_plane_levels[i] >= _axial_plane_levels[i + 1])
        mooseError("Ducted pin engineering unit must have axial plane levels defined in strictly "
                   "ascending order");

  // Check size of axial plane names
  if (_axial_plane_levels.size() != _axial_plane_names.size())
    mooseError("Size of axial plane levels must match size of axial plane names");

  // Check size of region ids
  unsigned int n_axial = _axial_plane_levels.size() + 1;
  if (_region_names.size() != n_axial)
    mooseError("Size of region IDs must be one larger than the number of axial levels");

  unsigned int n_radial = _ring_radii.size() + _duct_apothems.size() + 1;
  for (const auto & region_names_radial : _region_names)
    if (region_names_radial.size() != n_radial)
      mooseError("Size of each entry in region IDs must be one larger than the number of radial "
                 "zones in pin");

  _geometry_type = geometry_type;
}

std::unordered_map<std::string, AttributeVariant>
DuctedPinEngUnit::getAttributes() const
{
  std::unordered_map<std::string, AttributeVariant> attr_map{
      {"duct_apothems", _duct_apothems},
      {"ring_radii", _ring_radii},
      {"region_names", _region_names},
      {"geometry_type", getGeometryTypeString()}};
  if (_axial_plane_levels.size())
    attr_map["axial_plane_levels"] = _axial_plane_levels;
  return attr_map;
}

std::unique_ptr<CSGUniverseEngUnit>
DuctedPinEngUnit::clone() const
{
  return std::make_unique<DuctedPinEngUnit>(_name,
                                            getGeometryTypeString(),
                                            _ring_radii,
                                            _duct_apothems,
                                            _region_names,
                                            _axial_plane_levels,
                                            _axial_plane_names);
}

void
DuctedPinEngUnit::expandUnit()
{
  const auto has_axial_levels = _axial_plane_levels.size() > 0;
  unsigned int n_axial = _axial_plane_levels.size() + 1;
  unsigned int n_ring = _ring_radii.size();
  std::vector<std::reference_wrapper<const CSG::PinUniverseEngUnit>> pin_units_by_axial_region;

  // Define pin universe engineering units for each axial level
  if (n_ring)
  {
    for (const auto i : make_range(n_axial))
    {
      auto unit_name = _name + "_pin_unit";
      if (has_axial_levels)
        unit_name += "_axial_" + std::to_string(i);
      std::vector<std::string> pin_fill_mats;
      for (const auto j : make_range(n_ring + 1))
        pin_fill_mats.push_back(_region_names[i][j]);
      // Create a PinUniverseEngUnit engineering unit and add it to CSGBase
      std::unique_ptr<CSG::PinUniverseEngUnit> pin_ptr =
          std::make_unique<CSG::PinUniverseEngUnit>(unit_name, _ring_radii, pin_fill_mats);
      auto & pin_unit = _internal_base->addEngUnit(std::move(pin_ptr));
      pin_units_by_axial_region.push_back(pin_unit);
    }
  }

  // Define CSGNPolygonUnit representing each duct unit and define all radial regions
  // created by the ducts
  std::vector<CSG::CSGRegion> radial_regions;
  CSG::CSGRegion inner_region, outer_region, radial_region;
  std::vector<std::reference_wrapper<const CSG::CSGNPolygonUnit>> duct_units_by_radial_region;
  for (const auto i : index_range(_duct_apothems))
  {
    const auto unit_name = _name + "_radial_duct_" + std::to_string(i);
    const auto n_sides = (getGeometryTypeString() == "Hex") ? 6 : 4;
    std::unique_ptr<CSG::CSGNPolygonUnit> duct_ptr =
        std::make_unique<CSG::CSGNPolygonUnit>(unit_name, n_sides, _duct_apothems[i]);
    auto & duct_unit = _internal_base->addEngUnit(std::move(duct_ptr));
    duct_units_by_radial_region.push_back(duct_unit);

    if (i == 0)
    {
      // We are in the innermost radial region, the radial region is inner_region
      inner_region = -duct_unit;
      radial_region = inner_region;
    }
    else
    {
      // For all other regions, the radial region is the intersection of inner_region and
      // outer_region
      outer_region = ~inner_region;
      inner_region = -duct_unit;
      radial_region = inner_region & outer_region;
    }
    radial_regions.push_back(radial_region);
  }

  // Define outermost region
  radial_region = radial_regions.empty() ? outer_region : ~inner_region;
  radial_regions.push_back(radial_region);

  // Define all axial surfaces and regions
  std::vector<CSG::CSGRegion> axial_regions;
  std::vector<std::reference_wrapper<const CSG::CSGSurface>> surfaces_by_axial_region;
  if (has_axial_levels)
    for (const auto i : make_range(n_axial))
    {
      // Create the axial plane and add it to _internal_base
      if (i != _axial_plane_levels.size())
      {
        auto axial_level = _axial_plane_levels[i];
        auto axial_surf_name = _axial_plane_names[i];
        std::unique_ptr<CSG::CSGSurface> plane_surf_ptr =
            std::make_unique<CSG::CSGPlane>(axial_surf_name, 0, 0, 1, axial_level);
        const auto & plane_surf = _internal_base->addSurface(std::move(plane_surf_ptr));
        surfaces_by_axial_region.push_back(plane_surf);
      }

      // Define each axial region. This will be one more than the number of axial planes
      CSG::CSGRegion axial_region;
      const auto & current_surf = surfaces_by_axial_region.back().get();
      if (i == 0)
        // First axial region is the negative halfspace of the first plane
        axial_region = -current_surf;
      else if (i == _axial_plane_levels.size())
        // Last axial region is the positive halfspace of the last plane
        axial_region = +current_surf;
      else
      {
        // All other axial regions are the intersection of the negative halfspace
        // of the current plane and the positive halfspace of the previous plane
        const auto & prev_surf = surfaces_by_axial_region[i - 1].get();
        axial_region = -current_surf & +prev_surf;
      }
      axial_regions.push_back(axial_region);
    }

  // Define all cells within pin domain and add to root universe of engineering unit
  for (const auto i : index_range(radial_regions))
  {
    const unsigned int radial_index = _ring_radii.size() + i;
    for (const auto j : make_range(has_axial_levels ? axial_regions.size() : 1))
    {
      auto cell_region = radial_regions[i];
      auto cell_name = _name + "_cell_radial_" + std::to_string(i);
      if (has_axial_levels)
      {
        // update name and region with axial info only if extruded
        const auto axial_region = axial_regions[j];
        if (cell_region.getRegionType() != CSG::CSGRegion::RegionType::EMPTY)
          cell_region &= axial_region;
        else
          cell_region = axial_region;
        cell_name += "_axial_" + std::to_string(j);
      }
      if (i == 0 && n_ring > 0)
        // For first radial region where pin rings exist, we fill cell with pin universe unit
        _internal_base->createCell(cell_name, pin_units_by_axial_region[j], cell_region);
      else
      {
        // Otherwise, we fill the region with a material fill based on the region name
        const auto mat_name = _region_names[j][radial_index];
        _internal_base->createCell(cell_name, mat_name, cell_region);
      }
    }
  }
}

} // namespace CSG
