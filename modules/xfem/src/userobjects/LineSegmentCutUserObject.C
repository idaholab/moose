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
  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() != 4)
    mooseError("Length of LineSegmentCutUserObject cut_data must be 4");

  // Get scale and translate parameters if available
  std::vector<Real> scale;
  if (isParamValid("cut_scale"))
  {
    scale = getParam<std::vector<Real>>("cut_scale");
  }
  else
  {
    scale.push_back(1.0);
    scale.push_back(1.0);
  }

  std::vector<Real> trans;
  if (isParamValid("cut_translate"))
  {
    trans = getParam<std::vector<Real>>("cut_translate");
  }
  else
  {
    trans.push_back(0.0);
    trans.push_back(0.0);
  }

  // Assign translated and scaled cut_data to vars used to construct cuts
  Real x0 = (_cut_data[0] + trans[0]) * scale[0];
  Real y0 = (_cut_data[1] + trans[1]) * scale[1];
  Real x1 = (_cut_data[2] + trans[0]) * scale[0];
  Real y1 = (_cut_data[3] + trans[1]) * scale[1];

  _cut_line_start_points.push_back(Point(x0, y0, 0.0));
  _cut_line_end_points.push_back(Point(x1, y1, 0.0));
}

LineSegmentCutUserObject::~LineSegmentCutUserObject() {}
