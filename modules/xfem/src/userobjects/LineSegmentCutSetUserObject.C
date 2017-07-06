/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LineSegmentCutSetUserObject.h"

// MOOSE includes
#include "MooseError.h"

template <>
InputParameters
validParams<LineSegmentCutSetUserObject>()
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

LineSegmentCutSetUserObject::LineSegmentCutSetUserObject(const InputParameters & parameters)
  : GeometricCut2DUserObject(parameters), _cut_data(getParam<std::vector<Real>>("cut_data"))
{
  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() % 6 != 0)
    mooseError("Length of LineSegmentCutSetUserObject cut_data must be a multiple of 6.");

  unsigned int num_cuts = _cut_data.size() / 6;

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

  // Clear _start_times & _end_times vectors initialized from
  // time_start_cut & time_end_cut values
  _start_times.clear();
  _end_times.clear();

  for (unsigned int i = 0; i < num_cuts; ++i)
  {
    Real x0 = (_cut_data[i * 6 + 0] + trans[0]) * scale[0];
    Real y0 = (_cut_data[i * 6 + 1] + trans[1]) * scale[1];
    Real x1 = (_cut_data[i * 6 + 2] + trans[0]) * scale[0];
    Real y1 = (_cut_data[i * 6 + 3] + trans[1]) * scale[1];
    _cut_line_start_points.push_back(Point(x0, y0, 0.0));
    _cut_line_end_points.push_back(Point(x1, y1, 0.0));

    _start_times.push_back(_cut_data[i * 6 + 4]);
    _end_times.push_back(_cut_data[i * 6 + 5]);
  }
}

LineSegmentCutSetUserObject::~LineSegmentCutSetUserObject() {}
