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
      "value_name", "reporter value name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "x_coord_name",
      "reporter x-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "y_coord_name",
      "reporter y-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>(
      "z_coord_name",
      "reporter z-coordinate name.  This uses the reporter syntax <reporter>/<name>.");
  params.addParam<ReporterName>("point_name",
                                "reporter point name.  This uses the reporter syntax "
                                "<reporter>/<name>.");
  params.addParam<ReporterName>("weight_name",
                                "Name of vector-postprocessor or reporter vector containing "
                                "weights to scale value, default is assumed to be all 1s.");
  return params;
}

ReporterPointSource::ReporterPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _values(getReporterValue<std::vector<Real>>("value_name", REPORTER_MODE_REPLICATED)),
    _ones_vec(_values.size(), 1.0),
    _zeros_vec(_values.size(), 0.0),
    _zeros_pts(_values.size(), Point()),
    _coordx(isParamValid("x_coord_name")
                ? getReporterValue<std::vector<Real>>("x_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _coordy(isParamValid("y_coord_name")
                ? getReporterValue<std::vector<Real>>("y_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _coordz(isParamValid("z_coord_name")
                ? getReporterValue<std::vector<Real>>("z_coord_name", REPORTER_MODE_REPLICATED)
                : _zeros_vec),
    _point(isParamValid("point_name")
               ? getReporterValue<std::vector<Point>>("point_name", REPORTER_MODE_REPLICATED)
               : _zeros_pts),
    _weight(isParamValid("weight_name")
                ? getReporterValue<std::vector<Real>>("weight_name", REPORTER_MODE_REPLICATED)
                : _ones_vec)
{
  if (isParamValid("point_name") == (isParamValid("x_coord_name") && isParamValid("y_coord_name") &&
                                     isParamValid("z_coord_name")))
    paramError("Either supply x,y, and z reporters or a point reporter.");
}

void
ReporterPointSource::addPoints()
{
  const auto nval = _values.size();
  if (nval == 0)
    paramError("value_name", "Value vector must not be empty.");

  // resize these incase the values reporters changed size
  // this will only change data constructed to reference these
  _ones_vec.resize(nval, 1.0);
  _zeros_vec.resize(nval, 0.0);
  _zeros_pts.resize(nval, Point());

  errorCheck("x_coord_name", _coordx.size());
  errorCheck("y_coord_name", _coordy.size());
  errorCheck("z_coord_name", _coordz.size());
  errorCheck("weight_name", _weight.size());
  errorCheck("point_name", _point.size());

  if (isParamValid("point_name"))
  {
    for (const auto i : index_range(_values))
    {
      _point_to_index[_point[i]] = i;
      addPoint(_point[i], i);
    }
  }
  else
  {
    for (const auto i : index_range(_values))
    {
      const Point pt(_coordx[i], _coordy[i], _coordz[i]);
      _point_to_index[pt] = i;
      addPoint(pt, i);
    }
  }
}

Real
ReporterPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  std::size_t index = _point_to_index[_current_point];
  return -_test[_i][_qp] * _values[index] * _weight[index];
}

void
ReporterPointSource::errorCheck(const std::string & input_name, std::size_t reporterSize)
{
  const auto nval = _values.size();
  if (reporterSize != nval)
    paramError(input_name,
               "Number of ",
               input_name,
               " entries (",
               reporterSize,
               ") does not match number of entries read for value_name (",
               nval,
               ").");
}
