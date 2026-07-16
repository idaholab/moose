//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PinEngineeringUnit.h"
#include "CSGZCylinder.h"
#include "RGMBEngUnitUtils.h"
#include "CSGUtils.h"

namespace CSG
{

PinEngineeringUnit::PinEngineeringUnit(const std::string & name,
                                       const std::string & geometry_type,
                                       const std::vector<Real> & ring_radii,
                                       const std::vector<Real> & duct_apothems,
                                       const std::vector<std::vector<unsigned int>> & region_ids,
                                       const std::vector<Real> & axial_boundaries)
  : CSGCellEngUnit(name),
    _geometry_type(geometry_type),
    _ring_radii(ring_radii),
    _duct_apothems(duct_apothems),
    _region_ids(region_ids),
    _axial_boundaries(axial_boundaries)
{
  // Check radii in ascending order
  if (_ring_radii.size() > 1)
    for (const auto i : make_range(_ring_radii.size() - 1))
      if (_ring_radii[i] >= _ring_radii[i + 1])
        mooseError("Pin engineering unit must have ring radii defined in strictly ascending order");

  // Check duct apothems in ascending order
  if (_duct_apothems.size() > 1)
    for (const auto i : make_range(_duct_apothems.size() - 1))
      if (_duct_apothems[i] >= _duct_apothems[i + 1])
        mooseError(
            "Pin engineering unit must have duct apothems defined in strictly ascending order");

  // Check size of region ids
  unsigned int n_axial = _axial_boundaries.size() ? axial_boundaries.size() : 1;
  if (_region_ids.size() != n_axial)
    mooseError("Size of region IDs must match the number of axial zones in pin");

  unsigned int n_radial = _ring_radii.size() + _duct_apothems.size();
  for (const auto & region_ids_radial : _region_ids)
    if (region_ids_radial.size() != n_radial)
      mooseError("Size of entry in region IDs must match the number of radial zones in pin");

  if (_geometry_type != "Hex" && _geometry_type != "Square")
    mooseError("Invalid geometry type for PinEngineeringUnit");
}

std::unordered_map<std::string, AttributeVariant>
PinEngineeringUnit::getAttributes() const
{
  std::unordered_map<std::string, AttributeVariant> attr_map{{"duct_apothems", _duct_apothems},
                                                             {"ring_radii", _ring_radii},
                                                             {"region_ids", _region_ids},
                                                             {"geometry_type", _geometry_type}};
  if (_axial_boundaries.size())
    attr_map["axial_boundaries"] = _axial_boundaries;
  return attr_map;
}

std::unique_ptr<CSGCellEngUnit>
PinEngineeringUnit::clone() const
{
  return std::make_unique<PinEngineeringUnit>(
      _name, _geometry_type, _ring_radii, _duct_apothems, _region_ids, _axial_boundaries);
}

void
PinEngineeringUnit::expandUnit()
{
  unsigned int radial_index = 0;
  std::vector<std::vector<std::reference_wrapper<const CSG::CSGSurface>>> surfaces_by_radial_region;

  // Add surfaces corresponding to pin rings
  for (const auto & radius : _ring_radii)
  {
    const auto surf_name = _name + "_radial_ring_" + std::to_string(radial_index);
    std::unique_ptr<CSG::CSGSurface> ring_surf_ptr =
        std::make_unique<CSG::CSGZCylinder>(surf_name, 0, 0, radius);
    const auto & ring_surf = _internal_base->addSurface(std::move(ring_surf_ptr));
    surfaces_by_radial_region.push_back({ring_surf});
    ++radial_index;
  }

  // Add surfaces corresponding to pin ducts
  for (const auto & duct_apothem : _duct_apothems)
  {
    const auto & duct_surfaces = RGMBEngUnitUtils::getOuterRadialSurfacesForUnitCell(
        radial_index, _geometry_type, _name, duct_apothem, *_internal_base);
    surfaces_by_radial_region.push_back(duct_surfaces);
    ++radial_index;
  }

  // Define all radial regions
  std::vector<CSG::CSGRegion> radial_regions;
  CSG::CSGRegion inner_region, outer_region;
  for (const auto i : index_range(surfaces_by_radial_region))
  {
    const auto & radial_surfaces = surfaces_by_radial_region[i];
    CSG::CSGRegion radial_region;
    bool is_last_radial_region = i == surfaces_by_radial_region.size() - 1;
    if (inner_region.getRegionType() == CSG::CSGRegion::RegionType::EMPTY)
    {
      if (!is_last_radial_region)
      {
        // We are in the innermost radial region, the radial region is inner_region
        inner_region = CSGUtils::getInnerRegion(radial_surfaces, Point(0, 0, 0));
        radial_region = inner_region;
      }
    }
    else
    {
      // For all other regions, the radial region is the intersection of inner_region and
      // outer_region
      outer_region = ~inner_region;
      inner_region = CSGUtils::getInnerRegion(radial_surfaces, Point(0, 0, 0));
      radial_region = is_last_radial_region ? outer_region : (inner_region & outer_region);
    }
    radial_regions.push_back(radial_region);
  }

  // Define all axial surfaces and regions
  std::vector<CSG::CSGRegion> axial_regions;
  std::vector<std::reference_wrapper<const CSG::CSGSurface>> surfaces_by_axial_region;
  const auto extruded_pin = _axial_boundaries.size() > 0;
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
    for (const auto j : make_range(extruded_pin ? axial_regions.size() : 1))
    {
      auto cell_region = radial_regions[i];
      auto cell_name = _name + "_cell_radial_" + std::to_string(i);
      const auto region_id = _region_ids[j][i];
      const auto mat_name = "rgmb_region_" + std::to_string(region_id);
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
      _internal_base->createCell(cell_name, mat_name, cell_region, &pin_univ);
    }
  }

  // Create new cell to bound universe based on pin outer boundaries and add this cell to the root
  // universe
  auto pin_region = CSGUtils::getInnerRegion(surfaces_by_radial_region.back(), Point(0, 0, 0));
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
