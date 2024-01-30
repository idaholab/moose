//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructurePlate.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructurePlate);

InputParameters
HeatStructurePlate::validParams()
{
  InputParameters params = HeatStructureBase::validParams();

  params.addRequiredParam<std::vector<std::string>>("names", "Name of each transverse region");
  params.addRequiredParam<std::vector<Real>>("widths", "Width of each transverse region [m]");
  params.addRequiredParam<std::vector<unsigned int>>(
      "n_part_elems", "Number of elements of each transverse region");
  params.addDeprecatedParam<std::vector<std::string>>(
      "materials",
      "Material name for each transverse region",
      "HeatStructureMaterials are deprecated. Please make corresponding SolidProperties objects "
      "and replace the heat structure parameter 'materials' with the parameters 'solid_properties' "
      "and 'solid_properties_T_ref'. See heat structure documentation for more information.");
  params.addParam<std::vector<UserObjectName>>(
      "solid_properties", "Solid properties object name for each radial region");
  params.addParam<std::vector<Real>>(
      "solid_properties_T_ref",
      {},
      "Density reference temperatures for each radial region. This is required if "
      "'solid_properties' is provided. The density in each region will be a constant value "
      "computed by evaluating the density function at the reference temperature.");
  params.addParam<Real>("num_rods", 1.0, "Number of rods represented by this heat structure");
  params.addRequiredParam<Real>("depth", "Dimension of plate fuel in the third direction [m]");

  params.addClassDescription("Plate heat structure");

  return params;
}

HeatStructurePlate::HeatStructurePlate(const InputParameters & params)
  : HeatStructureBase(params), _depth(getParam<Real>("depth"))
{
  _names = getParam<std::vector<std::string>>("names");
  _n_regions = _names.size();
  for (unsigned int i = 0; i < _names.size(); i++)
    _name_index[_names[i]] = i;

  _material_names = isParamValid("materials") ? getParam<std::vector<std::string>>("materials")
                                              : std::vector<std::string>{};

  _width = getParam<std::vector<Real>>("widths");
  _total_width = std::accumulate(_width.begin(), _width.end(), 0.0);

  _n_part_elems = getParam<std::vector<unsigned int>>("n_part_elems");
  for (unsigned int i = 0; i < _n_part_elems.size(); i++)
    _total_elem_number += _n_part_elems[i];

  _num_rods = getParam<Real>("num_rods");

  if (_width.size() == _n_regions)
  {
    for (unsigned int i = 0; i < _n_regions; i++)
      _volume.push_back(_num_rods * _width[i] * _depth * _length);
  }
}

void
HeatStructurePlate::check() const
{
  HeatStructureBase::check();

  checkEqualSize<std::string, unsigned int>("names", "n_part_elems");
  checkEqualSize<std::string, Real>("names", "widths");
  if (isParamValid("materials"))
    checkEqualSize<std::string, std::string>("names", "materials");
  if (isParamValid("solid_properties"))
  {
    checkEqualSize<UserObjectName, std::string>("solid_properties", "names");
    checkEqualSize<UserObjectName, Real>("solid_properties", "solid_properties_T_ref");
  }
  checkMutuallyExclusiveParameters({"materials", "solid_properties"}, false);
}

Real
HeatStructurePlate::getUnitPerimeter(const ExternalBoundaryType & side) const
{
  switch (side)
  {
    case ExternalBoundaryType::OUTER:
    case ExternalBoundaryType::INNER:
      return _depth;

    case ExternalBoundaryType::START:
    case ExternalBoundaryType::END:
      return std::numeric_limits<Real>::quiet_NaN();
  }

  mooseError(name(), ": Unknown value of 'side' parameter.");
}

Real
HeatStructurePlate::computeRadialBoundaryArea(const Real & length, const Real & /*y*/) const
{
  return length * _depth;
}

Real
HeatStructurePlate::computeAxialBoundaryArea(const Real & y_min, const Real & y_max) const
{
  return (y_max - y_min) * _depth;
}
