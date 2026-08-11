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
#include "RGMBEngUnitUtils.h"
#include "CSGUtils.h"

namespace CSG
{

DuctedPinEngUnit::DuctedPinEngUnit(const std::string & name,
                                   const std::string & geometry_type,
                                   const std::vector<Real> & ring_radii,
                                   const std::vector<Real> & duct_apothems,
                                   const std::vector<std::vector<unsigned int>> & region_ids,
                                   const std::vector<Real> & axial_boundaries)
  : CSGCellEngUnit(name),
    _ring_radii(ring_radii),
    _duct_apothems(duct_apothems),
    _region_ids(region_ids),
    _axial_boundaries(axial_boundaries)
{
  // Check radii in ascending order
  if (_ring_radii.size() > 1)
    for (const auto i : make_range(_ring_radii.size() - 1))
      if (_ring_radii[i] >= _ring_radii[i + 1])
        mooseError(
            "Ducted pin engineering unit must have ring radii defined in strictly ascending order");

  // Check duct apothems in ascending order
  if (_duct_apothems.size() == 0)
    mooseError("At least one duct apothem needs to be defined in DuctedPinEngUnit");
  else if (_duct_apothems.size() > 1)
    for (const auto i : make_range(_duct_apothems.size() - 1))
      if (_duct_apothems[i] >= _duct_apothems[i + 1])
        mooseError("Ducted pin engineering unit must have duct apothems defined in strictly "
                   "ascending order");

  // Check size of region ids
  unsigned int n_axial = _axial_boundaries.size() ? axial_boundaries.size() : 1;
  if (_region_ids.size() != n_axial)
    mooseError("Size of region IDs must match the number of axial zones in pin");

  unsigned int n_radial = _ring_radii.size() + _duct_apothems.size();
  for (const auto & region_ids_radial : _region_ids)
    if (region_ids_radial.size() != n_radial)
      mooseError("Size of entry in region IDs must match the number of radial zones in pin");

  _geometry_type = geometry_type;
}

std::unordered_map<std::string, AttributeVariant>
DuctedPinEngUnit::getAttributes() const
{
  std::unordered_map<std::string, AttributeVariant> attr_map{
      {"duct_apothems", _duct_apothems},
      {"ring_radii", _ring_radii},
      {"region_ids", _region_ids},
      {"geometry_type", getGeometryTypeString()}};
  if (_axial_boundaries.size())
    attr_map["axial_boundaries"] = _axial_boundaries;
  return attr_map;
}

std::unique_ptr<CSGCellEngUnit>
DuctedPinEngUnit::clone() const
{
  return std::make_unique<DuctedPinEngUnit>(
      _name, getGeometryTypeString(), _ring_radii, _duct_apothems, _region_ids, _axial_boundaries);
}

void
DuctedPinEngUnit::expandUnit()
{
  const auto extruded_pin = _axial_boundaries.size() > 0;
  unsigned int n_axial = _axial_boundaries.size() ? _axial_boundaries.size() : 1;
  unsigned int n_ring = _ring_radii.size();
  std::vector<std::reference_wrapper<const CSG::PinUniverseEngUnit>> pin_units_by_axial_region;

  // Define pin universe engineering units for each axial level
  if (n_ring)
  {
    for (const auto i : make_range(n_axial))
    {
      auto unit_name = _name + "_pin_unit";
      if (extruded_pin)
        unit_name += "_axial_" + std::to_string(i);
      std::vector<std::string> pin_fill_mats;
      for (const auto j : make_range(n_ring + 1))
        pin_fill_mats.push_back("rgmb_region_" + std::to_string(_region_ids[i][j]));
      std::unique_ptr<CSG::PinUniverseEngUnit> pin_ptr =
          std::make_unique<CSG::PinUniverseEngUnit>(unit_name, _ring_radii, pin_fill_mats);
      auto & pin_unit = _internal_base->addEngUnit(std::move(pin_ptr));
      pin_units_by_axial_region.push_back(pin_unit);
    }
  }

  // Define CSGNPolygonUnit representing each duct unit and define all radial regions
  // created by the ducts
  std::vector<CSG::CSGRegion> radial_regions;
  CSG::CSGRegion inner_region, outer_region;
  std::vector<std::reference_wrapper<const CSG::CSGNPolygonUnit>> duct_units_by_radial_region;
  for (const auto i : index_range(_duct_apothems))
  {
    const auto unit_name = _name + "_radial_duct_" + std::to_string(i);
    const auto n_sides = (getGeometryTypeString() == "Hex") ? 6 : 4;
    std::unique_ptr<CSG::CSGNPolygonUnit> duct_ptr =
        std::make_unique<CSG::CSGNPolygonUnit>(unit_name, n_sides, _duct_apothems[i]);
    auto & duct_unit = _internal_base->addEngUnit(std::move(duct_ptr));
    duct_units_by_radial_region.push_back(duct_unit);

    CSG::CSGRegion radial_region;
    bool is_last_radial_region = i == _duct_apothems.size() - 1;
    if (i == 0)
    {
      if (!is_last_radial_region)
      {
        // We are in the innermost radial region, the radial region is inner_region
        inner_region = -duct_unit;
        radial_region = inner_region;
      }
    }
    else
    {
      // For all other regions, the radial region is the intersection of inner_region and
      // outer_region
      outer_region = ~inner_region;
      inner_region = -duct_unit;
      radial_region = is_last_radial_region ? outer_region : (inner_region & outer_region);
    }
    radial_regions.push_back(radial_region);
  }

  // Define all axial surfaces and regions
  std::vector<CSG::CSGRegion> axial_regions;
  std::vector<std::reference_wrapper<const CSG::CSGSurface>> surfaces_by_axial_region;
  if (extruded_pin)
  {
    surfaces_by_axial_region =
        RGMBEngUnitUtils::getAxialPlaneSurfaces(*_internal_base, _axial_boundaries);
    for (const auto i : make_range(surfaces_by_axial_region.size()))
      if (i != 0)
      {
        CSG::CSGRegion axial_region;
        const auto & lower_surf = surfaces_by_axial_region[i - 1].get();
        if (lower_surf != surfaces_by_axial_region.front())
          axial_region = +lower_surf;
        const auto & upper_surf = surfaces_by_axial_region[i].get();
        if (upper_surf != surfaces_by_axial_region.back())
        {
          if (axial_region.getRegionType() == CSG::CSGRegion::RegionType::EMPTY)
            axial_region = -upper_surf;
          else
            axial_region &= -upper_surf;
        }
        axial_regions.push_back(axial_region);
      }
  }

  // Define all cells within pin domain and add to separate universe
  const auto & pin_univ = _internal_base->createUniverse(_name + "_univ");
  for (const auto i : index_range(radial_regions))
  {
    const unsigned int radial_index = _ring_radii.size() + i;
    for (const auto j : make_range(extruded_pin ? axial_regions.size() : 1))
    {
      auto cell_region = radial_regions[i];
      auto cell_name = _name + "_cell_radial_" + std::to_string(i);
      if (extruded_pin)
      {
        // update name and region with axial info only if extruded
        const auto axial_region = axial_regions[j];
        if (axial_region.getRegionType() != CSG::CSGRegion::RegionType::EMPTY)
        {
          if (cell_region.getRegionType() != CSG::CSGRegion::RegionType::EMPTY)
            cell_region &= axial_region;
          else
            cell_region = axial_region;
        }
        cell_name += "_axial_" + std::to_string(j);
      }
      if (i == 0 && n_ring > 0)
        // For first radial region where pin rings exist, we fill cell with pin universe unit
        _internal_base->createCell(cell_name, pin_units_by_axial_region[j], cell_region, &pin_univ);
      else
      {
        // Otherwise, we fill the region with a material name based on region ID for region
        const auto region_id = _region_ids[j][radial_index];
        const auto mat_name = "rgmb_region_" + std::to_string(region_id);
        _internal_base->createCell(cell_name, mat_name, cell_region, &pin_univ);
      }
    }
  }

  // Create new cell to bound universe based on pin outer boundaries and add this cell to the root
  // universe
  auto pin_region = -(duct_units_by_radial_region.back());
  if (extruded_pin)
  {
    const auto & lowest_axial_surf = surfaces_by_axial_region.front().get();
    const auto & highest_axial_surf = surfaces_by_axial_region.back().get();
    auto axial_region = +lowest_axial_surf & -highest_axial_surf;
    pin_region &= axial_region;
  }
  _internal_base->createCell(_name + "_root_cell", pin_univ, pin_region);
}

} // namespace CSG
