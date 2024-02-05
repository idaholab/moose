//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureCylindrical.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureCylindrical);

InputParameters
HeatStructureCylindrical::validParams()
{
  InputParameters params = HeatStructureCylindricalBase::validParams();

  params.addRequiredParam<std::vector<std::string>>("names", "Name of each radial region");
  params.addRequiredParam<std::vector<Real>>("widths", "Width of each radial region [m]");
  params.addRequiredParam<std::vector<unsigned int>>("n_part_elems",
                                                     "Number of elements of each radial region");
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
  params.addParam<Real>("inner_radius", 0., "Inner radius of the heat structure [m]");

  params.addClassDescription("Cylindrical heat structure");

  return params;
}

HeatStructureCylindrical::HeatStructureCylindrical(const InputParameters & params)
  : HeatStructureCylindricalBase(params)
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

  _inner_radius = getParam<Real>("inner_radius");

  if (_width.size() == _n_regions)
  {
    std::vector<Real> r(_n_regions + 1, _inner_radius);
    for (unsigned int i = 0; i < _n_regions; i++)
    {
      r[i + 1] = r[i] + _width[i];
      _volume.push_back(_num_rods * M_PI * (r[i + 1] * r[i + 1] - r[i] * r[i]) * _length);
    }
  }
}

void
HeatStructureCylindrical::check() const
{
  HeatStructureCylindricalBase::check();

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
