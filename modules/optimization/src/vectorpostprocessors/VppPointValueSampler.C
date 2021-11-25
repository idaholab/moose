//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VppPointValueSampler.h"

#include <numeric>

registerMooseObject("MooseApp", VppPointValueSampler);

InputParameters
VppPointValueSampler::validParams()
{
  InputParameters params = PointSamplerBase::validParams();

  params.addClassDescription("Sample variables at xyz coordinates defined by reporter.");

  params.addParam<std::string>("reporter_name",
                               "The name of the reporter object name containing positions");
  params.addParam<std::string>(
      "x_coord_name", "measurement_xcoord", "name of vpp containing x coordinates.");
  params.addParam<std::string>(
      "y_coord_name", "measurement_ycoord", "name of vpp containing y coordinates.");
  params.addParam<std::string>(
      "z_coord_name", "measurement_zcoord", "name of vpp containing z coordinates.");

  MooseEnum sort_options("x y z id");
  params.set<MooseEnum>("sort_by") = "id";
  params.suppressParameter<MooseEnum>("sort_by");

  return params;
}

VppPointValueSampler::VppPointValueSampler(const InputParameters & parameters)
  : PointSamplerBase(parameters)
{
}

void
VppPointValueSampler::initialize()
{
  std::vector<Real> x_coord(getPointDataHelper("x_coord_name"));
  std::vector<Real> y_coord(getPointDataHelper("y_coord_name"));
  std::vector<Real> z_coord(getPointDataHelper("z_coord_name"));

  if (x_coord.size() != y_coord.size() || x_coord.size() != z_coord.size() || x_coord.size() == 0)
  {
    std::string errMsg = "The coordinate vectors are empty or a different size.  \n"
                         "x_coord size = " +
                         std::to_string(x_coord.size()) +
                         ";  y_coord size = " + std::to_string(y_coord.size()) +
                         ";  z_coord size = " + std::to_string(z_coord.size());
    mooseError(errMsg);
  }

  _points.clear();
  for (std::size_t i = 0; i < x_coord.size(); ++i)
  {
    Point pt(x_coord[i], y_coord[i], z_coord[i]);
    _points.push_back(pt);
  }

  // Generate new Ids if the point vector has grown (non-negative counting numbers)
  if (_points.size() > _ids.size())
  {
    auto old_size = _ids.size();
    _ids.resize(_points.size());
    std::iota(_ids.begin() + old_size, _ids.end(), old_size);
  }
  // Otherwise sync the ids array to be smaller if the point vector has been shrunk
  else if (_points.size() < _ids.size())
    _ids.resize(_points.size());

  PointSamplerBase::initialize();
}

std::vector<Real>
VppPointValueSampler::getPointDataHelper(const std::string & param)
{
  ReporterName rname =
      isParamValid("reporter_name")
          ? ReporterName(getParam<std::string>("reporter_name"), getParam<std::string>(param))
          : ReporterName(getParam<std::string>(param));
  return getReporterValueByName<std::vector<Real>>(rname, REPORTER_MODE_REPLICATED);
}
