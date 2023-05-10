//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterPointSource.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", ReporterPointSource);

InputParameters
ReporterPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();

  params.addClassDescription("Apply a point load defined by Reporter.");

  params.addParam<ReporterName>(
      "x_coord_name",
      "reporter x-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "y_coord_name",
      "reporter y-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "z_coord_name",
      "reporter z-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "value_name", "reporter value name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>("point_name",
                                "reporter point name.  This uses the reporter syntax "
                                "<reporter>/<name>.");
  return params;
}

ReporterPointSource::ReporterPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _values(getReporterValue<std::vector<Real>>("value_name", REPORTER_MODE_REPLICATED)),
    _x_coord(isParamValid("x_coord_name")
                 ? getReporterValue<std::vector<Real>>("x_coord_name", REPORTER_MODE_REPLICATED)
                 : _empty_vec),
    _y_coord(isParamValid("y_coord_name")
                 ? getReporterValue<std::vector<Real>>("y_coord_name", REPORTER_MODE_REPLICATED)
                 : _empty_vec),
    _z_coord(isParamValid("z_coord_name")
                 ? getReporterValue<std::vector<Real>>("z_coord_name", REPORTER_MODE_REPLICATED)
                 : _empty_vec),
    _point(isParamValid("point_name")
               ? getReporterValue<std::vector<Point>>("point_name", REPORTER_MODE_REPLICATED)
               : _empty_points)
{
  if (isParamValid("point_name") == (isParamValid("x_coord_name") && isParamValid("y_coord_name") &&
                                     isParamValid("z_coord_name")))
    paramError("Either supply x,y, and z reporters or a point reporter.");
}

void
ReporterPointSource::addPoints()
{
  if (!isParamValid("point_name"))
  {
    if (_values.size() != _x_coord.size() || _values.size() != _y_coord.size() ||
        _values.size() != _z_coord.size())
    {
      const std::string errMsg =
          "The value and coordinate vectors are a different size.  \n"
          "There must be one value per coordinate.  If the sizes are \n"
          "zero, the reporter or reporter may not have been initialized with data \n"
          "before the Dirac Kernel is called.  \n"
          "Try setting \"execute_on = timestep_begin\" in the reporter being read. \n"
          "value size = " +
          std::to_string(_values.size()) + ";  x_coord size = " + std::to_string(_x_coord.size()) +
          ";  y_coord size = " + std::to_string(_y_coord.size()) +
          ";  z_coord size = " + std::to_string(_z_coord.size());

      mooseError(errMsg);
    }

    _point_to_index.clear();
    for (std::size_t i = 0; i < _values.size(); ++i)
    {
      const Point pt(_x_coord[i], _y_coord[i], _z_coord[i]);
      _point_to_index[pt] = i;
      addPoint(pt, i);
    }
  }
  else
  {
    if (_values.size() != _point.size())
    {
      std::string errMsg =
          "The value and point vectors are a different size.  \n"
          "There must be one value per point.  If the sizes are \n"
          "zero, the reporter may not have been initialized with data \n"
          "before the Dirac Kernel is called.  \n"
          "Try setting \"execute_on = timestep_begin\" in the reporter being read. \n"
          "value size = " +
          std::to_string(_values.size()) + ";  point size = " + std::to_string(_point.size());

      mooseError(errMsg);
    }
    for (const auto i : index_range(_values))
    {
      _point_to_index[_point[i]] = i;
      addPoint(_point[i], i);
    }
  }
}

Real
ReporterPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _values[_point_to_index[_current_point]];
}
