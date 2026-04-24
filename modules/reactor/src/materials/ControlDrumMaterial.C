//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ControlDrumMaterial.h"

registerMooseObject("ReactorApp", ControlDrumMaterial);

InputParameters
ControlDrumMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<std::vector<Point>>("rotation_centers", "The centers of the rotation");

  params.addRequiredParam<std::vector<MooseFunctorName>>("rotation_angle_functors",
                                                         "The rotation angle as functor values");
  params.addParam<std::vector<Real>>(
      "rotation_angle_offsets", "Offsets of rotation angles corresponding to rotation centers");

  MooseEnum dir("x y z -x -y -z", "z");
  params.addParam<MooseEnum>("rotation_axis", dir, "The rotation axis");

  params.addRequiredParam<std::vector<Real>>(
      "segment_angles",
      "The covering angles in degree of all segments that sum to 360.\n"
      "Order by starting segment in a counter-clock-wise direction with respect to the rotation "
      "axis.\n"
      "All rotation centers share the same segment angles.");

  params.addRequiredParam<std::vector<std::vector<MaterialPropertyName>>>(
      "segment_material_properties",
      "Material properties for all the rotation segments corresponding to drum material "
      "properties");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "drum_material_properties", "Material property names for the drums");

  params.addClassDescription("Evaluate a material property based on the material properties of all "
                             "segments of a rotating drum.");
  return params;
}

ControlDrumMaterial::ControlDrumMaterial(const InputParameters & parameters)
  : Material(parameters),
    _rotation_centers(parameters.get<std::vector<Point>>("rotation_centers")),
    _rotation_axis(parameters.get<MooseEnum>("rotation_axis")),
    _plus(_rotation_axis == "x" || _rotation_axis == "y" || _rotation_axis == "z"),
    _rotation_offsets(isParamValid("rotation_angle_offsets")
                          ? parameters.get<std::vector<Real>>("rotation_angle_offsets")
                          : std::vector<Real>(_rotation_centers.size(), 0.0)),
    _segment_angles(parameters.get<std::vector<Real>>("segment_angles")),
    _n_segments(_segment_angles.size())
{
  if (_rotation_axis == "x" || _rotation_axis == "-x")
    _dir = 0;
  else if (_rotation_axis == "y" || _rotation_axis == "-y")
    _dir = 1;
  else
    _dir = 2;

  if ((_dir == 0 || _dir == 1) && _mesh.spatialDimension() < 3)
    parameters.paramError(
        "rotation_axis", _rotation_axis, " requires mesh spatial dimention to be three.");

  const auto funcs = parameters.get<std::vector<MooseFunctorName>>("rotation_angle_functors");
  for (const auto & func_name : funcs)
  {
    _rotation_functors.push_back(&getFunctor<Real>(func_name));
    // fixme: we should check whether the functor is constant in space, but
    // that requires resolving MOOSE Issue #32787.
  }

  if (_rotation_centers.size() != _rotation_functors.size())
  {
    if (_rotation_functors.size() == 1)
    {
      // all rotation centers share the same function
      _rotation_functors.resize(_rotation_centers.size(), _rotation_functors[0]);
    }
    else
      parameters.paramError("rotation_angle_function",
                            "Number of rotation angle functions must agree with "
                            "the size of 'rotation_centers' or be equal to one in case of all "
                            "rotation centers sharing "
                            "the same function");
  }
  if (_rotation_offsets.size() != _rotation_centers.size())
    parameters.paramError("rotation_angle_offsets",
                          "Size must be equal to the size of 'rotation_centers' ");

  if (_n_segments < 2)
    parameters.paramError("segment_angles", "At least 2 segments are required.");

  Real angle = 0;
  for (const auto & v : _segment_angles)
    angle += v;
  if (angle != 360)
    parameters.paramError("segment_angles", "Must sum to 360 degree.");

  for (const auto & c : _rotation_centers)
    if (c(_dir) != 0)
      parameters.paramError("rotation_centers",
                            "Must have zero coordinate in the rotation direction");

  const auto drum_props = getParam<std::vector<MaterialPropertyName>>("drum_material_properties");
  for (const auto & prop_name : drum_props)
    _drum_properties.push_back(&declareProperty<Real>(prop_name));

  const auto all_prop_names =
      getParam<std::vector<std::vector<MaterialPropertyName>>>("segment_material_properties");
  if (all_prop_names.size() != drum_props.size())
    paramError(
        "segment_material_properties",
        "Leading size of the 2D array must be equal to the size of 'drum_material_properties'");
  for (const auto & prop_names : all_prop_names)
  {
    if (prop_names.size() != _n_segments)
      paramError("segment_material_properties",
                 "Number of segment materal properties of one drum material property must be equal "
                 "to the number of segments (",
                 _n_segments,
                 ")");
    std::vector<const MaterialProperty<Real> *> props;
    for (const auto & prop_name : prop_names)
      props.push_back(&getMaterialPropertyByName<Real>(prop_name));
    _segment_properties.push_back(props);
  }
}

void
ControlDrumMaterial::computeQpProperties()
{
  // get the id of the rotation center the current element is rotating around
  unsigned int rotation_id = 0;
  Real min_dist = std::numeric_limits<Real>::max();
  auto p = _current_elem->vertex_average();
  p(_dir) = 0;
  for (const auto i : index_range(_rotation_centers))
  {
    auto c = _rotation_centers[i];
    Real dist = (p - c).norm();
    if (dist < min_dist)
    {
      rotation_id = i;
      min_dist = dist;
    }
  }

  // functor does not depend on state and element, but we pass them to meet the interface
  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem);
  Real rotation_angle =
      (*_rotation_functors[rotation_id])(elem_arg, state) + _rotation_offsets[rotation_id];

  // get the angle of the current quadrature point
  Point v = _q_point[_qp] - _rotation_centers[rotation_id];
  v(_dir) = 0;
  const Real vnorm = v.norm();
  if (vnorm != 0.0)
    v /= vnorm;

  // (-180, 180]
  Real ang = std::acos(v((_dir + 1) % 3)) * (180 / M_PI);
  if (v((_dir + 2) % 3) < 0)
    ang = -ang;

  // make sure returned angle is within [rotation_angle, rotation_angle+360)
  while (ang < rotation_angle)
    ang += 360;
  while (ang - 360 >= rotation_angle)
    ang -= 360;

  // rotation wrt current rotation_angle [0, 360)
  if (_plus)
    ang -= rotation_angle;
  else
  {
    // flip the angle
    if (ang == rotation_angle)
      ang = 0;
    else
      ang = rotation_angle + 360 - ang;
  }

  // find the segment where the qp is located
  unsigned int seg_id = 0;
  Real upper_angle = _segment_angles[0];
  while (ang > upper_angle)
  {
    ++seg_id;
    mooseAssert(seg_id < _n_segments, "Internal error: segement id");
    upper_angle += _segment_angles[seg_id];
  }

  // assign material properties
  // Note: we do not attempt adjust the value considering the weight with the location
  //       of the segment interface.
  for (const auto i : index_range(_drum_properties))
    (*_drum_properties[i])[_qp] = (*_segment_properties[i][seg_id])[_qp];
}
