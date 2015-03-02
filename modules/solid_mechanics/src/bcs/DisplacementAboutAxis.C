/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "DisplacementAboutAxis.h"
#include "Function.h"

template<>
InputParameters validParams<DisplacementAboutAxis>()
{
  InputParameters params = validParams<PresetNodalBC>();
  addDisplacementAboutAxisParams(params);
  params.addRequiredParam<int>("component","The component for the rotational displacement");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

void addDisplacementAboutAxisParams(InputParameters& params)
{
  MooseEnum units("degrees radians");
  params.addRequiredParam<FunctionName>("function", "The function providing the angle of rotation.");
  params.addRequiredParam<MooseEnum>("angle_units",units,"The units of the angle of rotation. Choices are:" + units.getRawNames());
  params.addRequiredParam<RealVectorValue>("axis_origin","Origin of the axis of rotation");
  params.addRequiredParam<RealVectorValue>("axis_direction","Direction of the axis of rotation");
}

DisplacementAboutAxis::DisplacementAboutAxis(const std::string & name, InputParameters parameters) :
    PresetNodalBC(name, parameters),
    _component(getParam<int>("component")),
    _func(getFunction("function")),
    _angle_units(getParam<MooseEnum>("angle_units")),
    _axis_origin(getParam<RealVectorValue>("axis_origin")),
    _axis_direction(getParam<RealVectorValue>("axis_direction"))
{
  if (_component < 0 || _component > 2)
  {
    std::stringstream errMsg;
    errMsg << "Invalid component given for "
           << name
           << ": "
           << _component
           << "." << std::endl;

    mooseError( errMsg.str() );
  }
}

void
DisplacementAboutAxis::initialSetup()
{
  calculateTransformationMatrices();
}

Real
DisplacementAboutAxis::computeQpValue()
{
  Point p(*_current_node);

  Real angle(_func.value(_t, *_current_node));
  if (_angle_units == "degrees")
    angle = angle*libMesh::pi/180.0;

  ColumnMajorMatrix p_old(4,1);
  p_old(0,0) = p(0);
  p_old(1,0) = p(1);
  p_old(2,0) = p(2);
  p_old(3,0) = 1;

  ColumnMajorMatrix p_new = rotateAroundAxis(p_old, angle);

  return p_new(_component,0)-p_old(_component,0);
}

ColumnMajorMatrix
DisplacementAboutAxis::rotateAroundAxis(const ColumnMajorMatrix & p0, const Real angle)
{
  ColumnMajorMatrix rotz(4,4);
  rotz(0,0) = cos(angle);
  rotz(0,1) = -sin(angle);
  rotz(0,2) = 0;
  rotz(0,3) = 0;
  rotz(1,0) = sin(angle);
  rotz(1,1) = cos(angle);
  rotz(1,2) = 0;
  rotz(1,3) = 0;
  rotz(2,0) = 0;
  rotz(2,1) = 0;
  rotz(2,2) = 1;
  rotz(2,3) = 0;
  rotz(3,0) = 0;
  rotz(3,1) = 0;
  rotz(3,2) = 0;
  rotz(3,3) = 1;

  ColumnMajorMatrix transform = _transformation_matrix_inv * rotz * _transformation_matrix;
  return transform * p0;
}

void
DisplacementAboutAxis::calculateTransformationMatrices()
{
  //These parts of the transformation matrix only depend on the axis of rotation:
  //calculate only once, at initialization

  Real a(_axis_direction(0));
  Real b(_axis_direction(1));
  Real c(_axis_direction(2));
  Real l(a*a+b*b+c*c);
  Real v(b*b+c*c);

  ColumnMajorMatrix transl(4,4);
  transl(0,0) = 1;
  transl(0,1) = 0;
  transl(0,2) = 0;
  transl(0,3) = -_axis_origin(0);
  transl(1,0) = 0;
  transl(1,1) = 1;
  transl(1,2) = 0;
  transl(1,3) = -_axis_origin(1);
  transl(2,0) = 0;
  transl(2,1) = 0;
  transl(2,2) = 1;
  transl(2,3) = -_axis_origin(2);
  transl(3,0) = 0;
  transl(3,1) = 0;
  transl(3,2) = 0;
  transl(3,3) = 1;
ColumnMajorMatrix rotx(4,4);
  rotx(0,0) = 1;
  rotx(0,1) = 0;
  rotx(0,2) = 0;
  rotx(0,3) = 0;
  rotx(1,0) = 0;
  rotx(1,1) = c/v;
  rotx(1,2) = -b/v;
  rotx(1,3) = 0;
  rotx(2,0) = 0;
  rotx(2,1) = b/v;
  rotx(2,2) = c/v;
  rotx(2,3) = 0;
  rotx(3,0) = 0;
  rotx(3,1) = 0;
  rotx(3,2) = 0;
  rotx(3,3) = 1;
  ColumnMajorMatrix roty(4,4);
  roty(0,0) = v/l;
  roty(0,1) = 0;
  roty(0,2) = -a/l;
  roty(0,3) = 0;
  roty(1,0) = 0;
  roty(1,1) = 1;
  roty(1,2) = 0;
  roty(1,3) = 0;
  roty(2,0) = a/l;
  roty(2,1) = 0;
  roty(2,2) = v/l;
  roty(2,3) = 0;
  roty(3,0) = 0;
  roty(3,1) = 0;
  roty(3,2) = 0;
  roty(3,3) = 1;

  ColumnMajorMatrix transl_inv(4,4);
  transl.inverse(transl_inv);
  ColumnMajorMatrix rotx_inv(4,4);
  rotx.inverse(rotx_inv);
  ColumnMajorMatrix roty_inv(4,4);
  roty.inverse(roty_inv);

  _transformation_matrix = roty * rotx * transl;
  _transformation_matrix_inv = transl_inv * rotx_inv * roty_inv;

}
