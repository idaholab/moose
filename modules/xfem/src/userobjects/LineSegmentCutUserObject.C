//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSegmentCutUserObject.h"

registerMooseObject("XFEMApp", LineSegmentCutUserObject);

InputParameters
LineSegmentCutUserObject::validParams()
{
  // Get input parameters from parent class
  InputParameters params = GeometricCut2DUserObject::validParams();

  // Add required parameters
  params.addRequiredParam<std::vector<Real>>("cut_data",
                                             "Vector of Real values providing cut information");
  // Add optional parameters
  params.addParam<std::vector<Real>>("cut_scale", "X,Y scale factors for geometric cuts");
  params.addParam<std::vector<Real>>("cut_translate", "X,Y translations for geometric cuts");
  // Class description
  params.addClassDescription("Creates a UserObject for a line segment cut on 2D meshes for XFEM");
  // Return the parameters
  return params;
}

LineSegmentCutUserObject::LineSegmentCutUserObject(const InputParameters & parameters)
  : GeometricCut2DUserObject(parameters), _cut_data(getParam<std::vector<Real>>("cut_data"))
{
  // Set up constant parameters
  const int cut_data_len = 4;

  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() != cut_data_len)
    mooseError("Length of LineSegmentCutUserObject cut_data must be 4");

  // Assign scale and translate parameters
  std::pair<Real, Real> scale;
  if (isParamValid("cut_scale"))
  {
    auto vec_scale = getParam<std::vector<Real>>("cut_scale");
    scale = std::make_pair(vec_scale[0], vec_scale[1]);
  }
  else
  {
    scale = std::make_pair(1.0, 1.0);
  }

  std::pair<Real, Real> trans;
  if (isParamValid("cut_translate"))
  {
    auto vec_trans = getParam<std::vector<Real>>("cut_translate");
    trans = std::make_pair(vec_trans[0], vec_trans[1]);
  }
  else
  {
    trans = std::make_pair(0.0, 0.0);
  }

  // Assign translated and scaled cut_data to vars used to construct cuts
  Real x0 = (_cut_data[0] + trans.first) * scale.first;
  Real y0 = (_cut_data[1] + trans.second) * scale.second;
  Real x1 = (_cut_data[2] + trans.first) * scale.first;
  Real y1 = (_cut_data[3] + trans.second) * scale.second;

  _cut_line_endpoints.push_back(std::make_pair(Point(x0, y0, 0.0), Point(x1, y1, 0.0)));

  if (_cut_line_endpoints.size() != _cut_time_ranges.size())
    mooseError("Number of start/end times must match number of cut line endpoint sets");
}

const std::vector<Point>
LineSegmentCutUserObject::getCrackFrontPoints(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackFrontPoints() is not implemented for this object.");
}

const std::vector<RealVectorValue>
LineSegmentCutUserObject::getCrackPlaneNormals(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackPlaneNormals() is not implemented for this object.");
}
