//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CylindricalAverage.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", CylindricalAverage);

InputParameters
CylindricalAverage::validParams()
{
  InputParameters params = SpatialAverageBase::validParams();
  params.addRequiredParam<Point>("cylinder_axis", "Vector along cylinder coordinate axis");
  params.addClassDescription("Compute a cylindrical average of a variableas a function of radius "
                             "throughout the simulation domain.");
  return params;
}

CylindricalAverage::CylindricalAverage(const InputParameters & parameters)
  : SpatialAverageBase(parameters),
    _cyl_axis(getParam<Point>("cylinder_axis")),
    _cyl_axis_norm(_cyl_axis.norm())
{
}

Real
CylindricalAverage::computeDistance()
{
  // angle between cyl_axis and origin-to-q_point
  Point oqp = _q_point[_qp] - _origin;
  Real norm_oqp = oqp.norm();
  Real cos_theta = oqp * _cyl_axis / norm_oqp / _cyl_axis_norm;

  // the distance is the sine times the length of the hypotenuse == norm_oqp
  return norm_oqp * std::sqrt(1 - cos_theta * cos_theta);
}
