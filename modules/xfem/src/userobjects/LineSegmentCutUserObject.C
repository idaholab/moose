/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LineSegmentCutUserObject.h"

template <>
InputParameters
validParams<LineSegmentCutUserObject>()
{
  // Get input parameters from parent class
  InputParameters params = validParams<GeometricCut2DUserObject>();

  // Add required parameters
  params.addRequiredParam<std::vector<Real>>(
      "start_point",
      "Vector of Real values for X and Y coordinates of the starting point of the line segment");
  params.addRequiredParam<std::vector<Real>>(
      "end_point",
      "Vector of Real values for X and Y coordinates of the ending point of the line segment");
  // Add optional parameters
  params.addParam<std::vector<Real>>("cut_scale", "X,Y scale factors for geometric cuts");
  params.addParam<std::vector<Real>>("cut_translate", "X,Y translations for geometric cuts");
  // Class description
  params.addClassDescription("Creates a UserObject for a line segment cut on 2D meshes for XFEM");
  // Return the parameters
  return params;
}

LineSegmentCutUserObject::LineSegmentCutUserObject(const InputParameters & parameters)
  : GeometricCut2DUserObject(parameters),
    _start_point(getParam<std::vector<Real>>("start_point")),
    _end_point(getParam<std::vector<Real>>("end_point"))
{
  // Set up constant parameters
  const int point_data_len = 2;

  // Throw error if length of endpoints is incorrect
  if (_start_point.size() != point_data_len)
    mooseError("Length of LineSegmentCutUserObject start_point must be 2!");
  if (_end_point.size() != point_data_len)
    mooseError("Length of LineSegmentCutUserObject end_point must be 2!");

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

  // Assign translated and scaled cut data to vars used to construct cuts
  Real x0 = (_start_point[0] + trans.first) * scale.first;
  Real y0 = (_start_point[1] + trans.second) * scale.second;
  Real x1 = (_end_point[0] + trans.first) * scale.first;
  Real y1 = (_end_point[1] + trans.second) * scale.second;

  _cut_line_endpoints.push_back(std::make_pair(Point(x0, y0, 0.0), Point(x1, y1, 0.0)));
}
