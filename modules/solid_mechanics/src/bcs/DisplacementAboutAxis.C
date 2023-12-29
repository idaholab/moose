//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplacementAboutAxis.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", DisplacementAboutAxis);

InputParameters
DisplacementAboutAxis::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addClassDescription("Implements a boundary condition that enforces rotational"
                             "displacement around an axis on a boundary");
  addDisplacementAboutAxisParams(params);
  params.addRequiredParam<int>("component", "The component for the rotational displacement");
  params.set<bool>("use_displaced_mesh") = false;
  params.set<bool>("preset") = true;
  return params;
}

void
addDisplacementAboutAxisParams(InputParameters & params)
{
  MooseEnum units("degrees radians");
  params.addRequiredParam<FunctionName>(
      "function", "The function providing the total angle of rotation or the angular velocity.");
  params.addRequiredParam<MooseEnum>("angle_units",
                                     units,
                                     "The units of the angle of rotation. Choices are:" +
                                         units.getRawNames());
  params.addRequiredParam<RealVectorValue>("axis_origin", "Origin of the axis of rotation");
  params.addRequiredParam<RealVectorValue>("axis_direction", "Direction of the axis of rotation");
  params.addParam<bool>(
      "angular_velocity", false, "If true interprets the function value as an angular velocity");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
}

DisplacementAboutAxis::DisplacementAboutAxis(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _component(getParam<int>("component")),
    _func(getFunction("function")),
    _angle_units(getParam<MooseEnum>("angle_units")),
    _axis_origin(getParam<RealVectorValue>("axis_origin")),
    _axis_direction(getParam<RealVectorValue>("axis_direction")),
    _ndisp(coupledComponents("displacements")),
    _disp_old(_ndisp),
    _angular_velocity(getParam<bool>("angular_velocity"))
{
  if (_component < 0 || _component > 2)
    mooseError("Invalid component given for ", name(), ": ", _component, ".");

  if (_axis_direction.norm() == 0.)
    mooseError("Please specify a non-zero direction vector for the axis_direction in ", name());
}

void
DisplacementAboutAxis::initialSetup()
{
  calculateUnitDirectionVector();
  calculateTransformationMatrices();

  if (_angular_velocity)
    for (unsigned int i = 0; i < _ndisp; ++i)
      _disp_old[i] = &coupledDofValuesOld("displacements", i);
  else
    for (unsigned int i = 0; i < _ndisp; ++i)
      _disp_old[i] = nullptr;
}

Real
DisplacementAboutAxis::computeQpValue()
{
  Point p(*_current_node);

  Real angle(_func.value(_t, *_current_node));
  if (_angle_units == "degrees")
    angle = angle * libMesh::pi / 180.0;

  if (_angular_velocity)
    angle *= _dt;

  ColumnMajorMatrix p_old(4, 1);
  p_old(0, 0) = p(0);
  p_old(1, 0) = p(1);
  p_old(2, 0) = p(2);
  p_old(3, 0) = 1;

  if (_angular_velocity)
    for (unsigned int i = 0; i < _ndisp; i++)
      p_old(i, 0) += (*_disp_old[i])[_qp];

  ColumnMajorMatrix p_new = rotateAroundAxis(p_old, angle);

  return p_new(_component, 0) - p(_component);
}

ColumnMajorMatrix
DisplacementAboutAxis::rotateAroundAxis(const ColumnMajorMatrix & p0, const Real angle)
{
  ColumnMajorMatrix rotate_about_z(4, 4);
  rotate_about_z(0, 0) = cos(angle);
  rotate_about_z(0, 1) = -sin(angle);
  rotate_about_z(0, 2) = 0;
  rotate_about_z(0, 3) = 0;
  rotate_about_z(1, 0) = sin(angle);
  rotate_about_z(1, 1) = cos(angle);
  rotate_about_z(1, 2) = 0;
  rotate_about_z(1, 3) = 0;
  rotate_about_z(2, 0) = 0;
  rotate_about_z(2, 1) = 0;
  rotate_about_z(2, 2) = 1;
  rotate_about_z(2, 3) = 0;
  rotate_about_z(3, 0) = 0;
  rotate_about_z(3, 1) = 0;
  rotate_about_z(3, 2) = 0;
  rotate_about_z(3, 3) = 1;

  ColumnMajorMatrix transform =
      _transformation_matrix_inv * rotate_about_z * _transformation_matrix;
  return transform * p0;
}

void
DisplacementAboutAxis::calculateUnitDirectionVector()
{
  Real magnitude = _axis_direction.norm();
  _axis_direction /= magnitude;
}

void
DisplacementAboutAxis::calculateTransformationMatrices()
{
  // These parts of the transformation matrix only depend on the axis of rotation:

  Real length = _axis_direction.norm_sq();
  Real v = _axis_direction(1) * _axis_direction(1) + _axis_direction(2) * _axis_direction(2);

  ColumnMajorMatrix transl(4, 4);
  transl(0, 0) = 1;
  transl(0, 1) = 0;
  transl(0, 2) = 0;
  transl(0, 3) = -_axis_origin(0);
  transl(1, 0) = 0;
  transl(1, 1) = 1;
  transl(1, 2) = 0;
  transl(1, 3) = -_axis_origin(1);
  transl(2, 0) = 0;
  transl(2, 1) = 0;
  transl(2, 2) = 1;
  transl(2, 3) = -_axis_origin(2);
  transl(3, 0) = 0;
  transl(3, 1) = 0;
  transl(3, 2) = 0;
  transl(3, 3) = 1;

  ColumnMajorMatrix rotate_about_x(4, 4);
  rotate_about_x(0, 0) = 1;
  rotate_about_x(0, 1) = 0;
  rotate_about_x(0, 2) = 0;
  rotate_about_x(0, 3) = 0;
  rotate_about_x(1, 0) = 0;
  rotate_about_x(1, 1) = _axis_direction(2) / v;
  rotate_about_x(1, 2) = -_axis_direction(1) / v;
  rotate_about_x(1, 3) = 0;
  rotate_about_x(2, 0) = 0;
  rotate_about_x(2, 1) = _axis_direction(1) / v;
  rotate_about_x(2, 2) = _axis_direction(2) / v;
  rotate_about_x(2, 3) = 0;
  rotate_about_x(3, 0) = 0;
  rotate_about_x(3, 1) = 0;
  rotate_about_x(3, 2) = 0;
  rotate_about_x(3, 3) = 1;

  ColumnMajorMatrix rotate_about_y(4, 4);
  rotate_about_y(0, 0) = v / length;
  rotate_about_y(0, 1) = 0;
  rotate_about_y(0, 2) = -_axis_direction(0) / length;
  rotate_about_y(0, 3) = 0;
  rotate_about_y(1, 0) = 0;
  rotate_about_y(1, 1) = 1;
  rotate_about_y(1, 2) = 0;
  rotate_about_y(1, 3) = 0;
  rotate_about_y(2, 0) = _axis_direction(0) / length;
  rotate_about_y(2, 1) = 0;
  rotate_about_y(2, 2) = v / length;
  rotate_about_y(2, 3) = 0;
  rotate_about_y(3, 0) = 0;
  rotate_about_y(3, 1) = 0;
  rotate_about_y(3, 2) = 0;
  rotate_about_y(3, 3) = 1;

  ColumnMajorMatrix transl_inv(4, 4);
  transl.inverse(transl_inv);
  ColumnMajorMatrix rotx_inv(4, 4);
  rotate_about_x.inverse(rotx_inv);
  ColumnMajorMatrix roty_inv(4, 4);
  rotate_about_y.inverse(roty_inv);

  _transformation_matrix = rotate_about_y * rotate_about_x * transl;
  _transformation_matrix_inv = transl_inv * rotx_inv * roty_inv;
}
