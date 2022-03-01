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

  params.addRequiredParam<ReporterName>(
      "x_coord_name",
      "reporter x-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "y_coord_name",
      "reporter y-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "z_coord_name",
      "reporter z-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "value_name", "reporter value name.  This uses the reporter syntax <reporter>/<name>.");

  return params;
}

ReporterPointSource::ReporterPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _values(getReporterValue<std::vector<Real>>("value_name", REPORTER_MODE_REPLICATED)),
    _x_coord(getReporterValue<std::vector<Real>>("x_coord_name", REPORTER_MODE_REPLICATED)),
    _y_coord(getReporterValue<std::vector<Real>>("y_coord_name", REPORTER_MODE_REPLICATED)),
    _z_coord(getReporterValue<std::vector<Real>>("z_coord_name", REPORTER_MODE_REPLICATED))
{
}

void
ReporterPointSource::addPoints()
{
  if (_values.size() != _x_coord.size() || _values.size() != _y_coord.size() ||
      _values.size() != _z_coord.size())
  {
    std::string errMsg =
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
    Point pt(_x_coord[i], _y_coord[i], _z_coord[i]);
    _point_to_index[pt] = i;
    addPoint(pt, i);
  }
}

Real
ReporterPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _values[_point_to_index[_current_point]];
}
