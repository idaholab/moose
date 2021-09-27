//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpatialUserObjectVectorPostprocessor.h"
#include "DelimitedFileReader.h"
#include "UserObjectInterface.h"

registerMooseObject("MooseApp", SpatialUserObjectVectorPostprocessor);

defineLegacyParams(SpatialUserObjectVectorPostprocessor);

InputParameters
SpatialUserObjectVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<UserObjectName>("userobject",
                                          "The userobject whose values are to be reported");
  params.addParam<std::vector<Point>>("points",
                                      "Computations will be lumped into values at these points.");
  params.addParam<FileName>("points_file",
                            "A filename that should be looked in for points. Each "
                            "set of 3 values in that file will represent a Point.  "
                            "This and 'points' cannot be both supplied.");
  params.addParam<bool>(
      "use_points_from_uo", false, "Whether to obtain the points directly from the user object");
  params.addClassDescription("Outputs the values of a spatial user object in the order "
                             "of the specified spatial points");

  return params;
}

SpatialUserObjectVectorPostprocessor::SpatialUserObjectVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _uo_vec(declareVector(MooseUtils::shortName(parameters.get<std::string>("_object_name")))),
    _uo(getUserObject<UserObject>("userobject")),
    _use_points_from_uo(getParam<bool>("use_points_from_uo"))
{
  fillPoints();
  _uo_vec.resize(_points.size());
}

void
SpatialUserObjectVectorPostprocessor::fillPoints()
{
  if (_use_points_from_uo)
  {
    if (isParamValid("points") || isParamValid("points_file"))
      mooseWarning("When 'use_points_from_uo' is true, the 'points' and 'points_file' parameters "
                   "are ignored");

    _points = _uo.spatialPoints();
  }
  else
  {
    if (isParamValid("points") && isParamValid("points_file"))
      mooseError(name(), ": Both 'points' and 'points_file' cannot be specified simultaneously.");

    if (isParamValid("points"))
    {
      _points = getParam<std::vector<Point>>("points");
    }
    else if (isParamValid("points_file"))
    {
      const FileName & points_file = getParam<FileName>("points_file");

      MooseUtils::DelimitedFileReader file(points_file, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();
      _points = file.getDataAsPoints();
    }
    else
      mooseError(name(), ": You need to supply either 'points' or 'points_file' parameter.");
  }
}

void
SpatialUserObjectVectorPostprocessor::initialize()
{
  _uo_vec.clear();
}

void
SpatialUserObjectVectorPostprocessor::execute()
{
  for (const auto & pt : _points)
    _uo_vec.push_back(_uo.spatialValue(pt));
}
