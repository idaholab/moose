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
  params.addParam<std::vector<std::string>>("materials",
                                            "Material name for each transverse region");
  params.addParam<Real>("num_rods", 1.0, "Number of rods represented by this heat structure");
  params.addRequiredParam<Real>("depth", "Dimension of plate fuel in the third direction [m]");

  params.addClassDescription("Plate heat structure");

  return params;
}

HeatStructurePlate::HeatStructurePlate(const InputParameters & params)
  : HeatStructureBase(params), _depth(getParam<Real>("depth"))
{
  _names = getParam<std::vector<std::string>>("names");
  _number_of_hs = _names.size();
  for (unsigned int i = 0; i < _names.size(); i++)
    _name_index[_names[i]] = i;

  _material_names = getParam<std::vector<std::string>>("materials");

  _width = getParam<std::vector<Real>>("widths");
  _total_width = std::accumulate(_width.begin(), _width.end(), 0.0);

  _n_part_elems = getParam<std::vector<unsigned int>>("n_part_elems");
  for (unsigned int i = 0; i < _n_part_elems.size(); i++)
    _total_elem_number += _n_part_elems[i];

  _num_rods = getParam<Real>("num_rods");

  if (_width.size() == _number_of_hs)
  {
    for (unsigned int i = 0; i < _number_of_hs; i++)
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
}

Real
HeatStructurePlate::getUnitPerimeter(const HeatStructureSideType & side) const
{
  switch (side)
  {
    case HeatStructureSideType::OUTER:
    case HeatStructureSideType::INNER:
      return _depth;

    case HeatStructureSideType::START:
    case HeatStructureSideType::END:
      return std::numeric_limits<Real>::quiet_NaN();
  }

  mooseError(name(), ": Unknown value of 'side' parameter.");
}
