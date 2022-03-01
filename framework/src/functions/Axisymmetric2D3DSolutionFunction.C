//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "Axisymmetric2D3DSolutionFunction.h"
#include "SolutionUserObject.h"

registerMooseObject("MooseApp", Axisymmetric2D3DSolutionFunction);

InputParameters
Axisymmetric2D3DSolutionFunction::validParams()
{
  // Get the Function input parameters
  InputParameters params = Function::validParams();

  // Add parameters specific to this object
  params.addRequiredParam<UserObjectName>("solution",
                                          "The SolutionUserObject to extract data from.");
  params.addParam<std::vector<std::string>>(
      "from_variables",
      "The names of the variables in the file that are to be extracted, in x, y "
      "order if they are vector components");

  params.addParam<Real>(
      "scale_factor",
      1.0,
      "Scale factor (a) to be applied to the solution (x): ax+b, where b is the 'add_factor'");
  params.addParam<Real>(
      "add_factor",
      0.0,
      "Add this value (b) to the solution (x): ax+b, where a is the 'scale_factor'");
  params.addParam<Real>("axial_dimension_ratio",
                        1.0,
                        "Ratio of the axial dimension in the 3d model to that in the 2d model. "
                        "Optionally permits the 3d model to be larger than the 2d model in that "
                        "dimension, and scales vector solutions in that direction by this factor.");

  params.addParam<RealVectorValue>("2d_axis_point1",
                                   RealVectorValue(0, 0, 0),
                                   "Start point for axis of symmetry for the 2d model");
  params.addParam<RealVectorValue>("2d_axis_point2",
                                   RealVectorValue(0, 1, 0),
                                   "End point for axis of symmetry for the 2d model");
  params.addParam<RealVectorValue>("3d_axis_point1",
                                   RealVectorValue(0, 0, 0),
                                   "Start point for axis of symmetry for the 3d model");
  params.addParam<RealVectorValue>("3d_axis_point2",
                                   RealVectorValue(0, 1, 0),
                                   "End point for axis of symmetry for the 3d model");

  params.addParam<unsigned int>("component",
                                "Component of the variable to be computed if it is a vector");

  params.addClassDescription("Function for reading a 2D axisymmetric solution from file and "
                             "mapping it to a 3D Cartesian model");

  return params;
}

Axisymmetric2D3DSolutionFunction::Axisymmetric2D3DSolutionFunction(
    const InputParameters & parameters)
  : Function(parameters),
    _solution_object_ptr(NULL),
    _scale_factor(getParam<Real>("scale_factor")),
    _add_factor(getParam<Real>("add_factor")),
    _axial_dim_ratio(getParam<Real>("axial_dimension_ratio")),
    _2d_axis_point1(getParam<RealVectorValue>("2d_axis_point1")),
    _2d_axis_point2(getParam<RealVectorValue>("2d_axis_point2")),
    _3d_axis_point1(getParam<RealVectorValue>("3d_axis_point1")),
    _3d_axis_point2(getParam<RealVectorValue>("3d_axis_point2")),
    _has_component(isParamValid("component")),
    _component(_has_component ? getParam<unsigned int>("component") : 99999),
    _var_names(getParam<std::vector<std::string>>("from_variables"))
{
  if (_has_component && _var_names.size() != 2)
    mooseError("Must supply names of 2 variables in 'from_variables' if 'component' is specified");
  else if (!_has_component && _var_names.size() == 2)
    mooseError("Must supply 'component' if 2 variables specified in 'from_variables'");
  else if (_var_names.size() > 2)
    mooseError("If 'from_variables' is specified, it must have either 1 (scalar) or 2 (vector "
               "components) variables");

  Point zero;
  Point unit_vec_y;
  unit_vec_y(1) = 1;
  if (_2d_axis_point1 == zero && _2d_axis_point2 == unit_vec_y && _3d_axis_point1 == zero &&
      _3d_axis_point2 == unit_vec_y)
    _default_axes = true;
  else
    _default_axes = false;

  if (_3d_axis_point1.relative_fuzzy_equals(_3d_axis_point2))
    mooseError("3d_axis_point1 and 3d_axis_point2 must be different points");
  if (_2d_axis_point1.relative_fuzzy_equals(_2d_axis_point2))
    mooseError("2d_axis_point1 and 2d_axis_point2 must be different points");
}

void
Axisymmetric2D3DSolutionFunction::initialSetup()
{
  // Get a pointer to the SolutionUserObject. A pointer is used because the UserObject is not
  // available during the
  // construction of the function
  _solution_object_ptr = &getUserObject<SolutionUserObject>("solution");

  // If 'from_variable' is not specified, get the value from the SolutionUserObject
  if (_var_names.size() == 0)
  {
    // Get all the variables from the SolutionUserObject
    const std::vector<std::string> & vars = _solution_object_ptr->variableNames();

    // If there are more than one, throw an error
    if (vars.size() > 1)
      mooseError("If the SolutionUserObject contains multiple variables, the variable name must be "
                 "specified in the input file with 'from_variables'");

    // Define the variable
    _var_names.push_back(vars[0]);
  }
  if (_2d_axis_point1(2) != 0)
    mooseError("3rd component of 2d_axis_point1 must be zero");
  if (_2d_axis_point2(2) != 0)
    mooseError("3rd component of 2d_axis_point2 must be zero");

  _solution_object_var_indices.resize(_var_names.size());
  for (unsigned int i = 0; i < _var_names.size(); ++i)
    _solution_object_var_indices[i] = _solution_object_ptr->getLocalVarIndex(_var_names[i]);
}

Real
Axisymmetric2D3DSolutionFunction::value(Real t, const Point & p) const
{
  Point xypoint;
  Point r_dir_2d;
  Point z_dir_2d;
  Point r_dir_3d;
  Point z_dir_3d;
  bool r_gt_zero = false;

  if (_default_axes)
  {
    r_dir_2d(0) = 1;
    z_dir_2d(1) = 1;
    r_dir_3d = p;
    r_dir_3d(1) = 0;
    Real r = r_dir_3d.norm();
    if (MooseUtils::absoluteFuzzyGreaterThan(r, 0.0))
    {
      r_gt_zero = true;
      r_dir_3d /= r;
    }
    z_dir_3d(1) = 1;
    xypoint(0) = std::sqrt(p(0) * p(0) + p(2) * p(2));
    xypoint(1) = p(1) / _axial_dim_ratio;
  }
  else
  {
    // Find the r, z coordinates of the point in the 3D model relative to the 3D axis
    z_dir_3d = _3d_axis_point2 - _3d_axis_point1;
    z_dir_3d /= z_dir_3d.norm();
    Point v3dp1p(p - _3d_axis_point1);
    Real z = z_dir_3d * v3dp1p;
    Point axis_proj = _3d_axis_point1 + z * z_dir_3d; // projection of point onto axis
    Point axis_proj_to_p = p - axis_proj;
    Real r = axis_proj_to_p.norm();
    if (MooseUtils::absoluteFuzzyGreaterThan(r, 0.0))
    {
      r_gt_zero = true;
      r_dir_3d = axis_proj_to_p / r;
    }

    // Convert point in r, z coordinates into x, y coordinates
    z_dir_2d = _2d_axis_point2 - _2d_axis_point1;
    z_dir_2d /= z_dir_2d.norm();
    Point out_of_plane_vec(0, 0, 1);
    r_dir_2d = z_dir_2d.cross(out_of_plane_vec);
    r_dir_2d /= r_dir_2d.norm(); // size should be 1, maybe this isn't necessary
    xypoint = _2d_axis_point1 + z / _axial_dim_ratio * z_dir_2d + r * r_dir_2d;
  }

  Real val;
  if (_has_component)
  {
    Real val_x = _solution_object_ptr->pointValue(t, xypoint, _solution_object_var_indices[0]);
    Real val_y = _solution_object_ptr->pointValue(t, xypoint, _solution_object_var_indices[1]);

    // val_vec_rz contains the value vector converted from x,y to r,z coordinates
    Point val_vec_rz;
    val_vec_rz(0) = r_dir_2d(0) * val_x + r_dir_2d(1) * val_y;
    val_vec_rz(1) = z_dir_2d(0) * val_x + z_dir_2d(1) * val_y;
    if (!r_gt_zero && !MooseUtils::absoluteFuzzyEqual(val_vec_rz(0), 0.0))
      mooseError("In Axisymmetric2D3DSolutionFunction r=0 and r component of value vector != 0");
    Point val_vec_3d = val_vec_rz(0) * r_dir_3d + val_vec_rz(1) * z_dir_3d;

    val = val_vec_3d(_component);
  }
  else
    val = _solution_object_ptr->pointValue(t, xypoint, _solution_object_var_indices[0]);

  return _scale_factor * val + _add_factor;
}
