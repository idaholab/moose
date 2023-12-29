//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DomainIntegralQFunction.h"

registerMooseObject("TensorMechanicsApp", DomainIntegralQFunction);

InputParameters
DomainIntegralQFunction::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the q-function for a segment along the crack front, used in "
                             "the calculation of the J-integral");
  params.addRequiredParam<Real>("j_integral_radius_inner", "Radius for J-Integral calculation");
  params.addRequiredParam<Real>("j_integral_radius_outer", "Radius for J-Integral calculation");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addParam<unsigned int>(
      "crack_front_point_index",
      "The index of the point on the crack front corresponding to this q function");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

DomainIntegralQFunction::DomainIntegralQFunction(const InputParameters & parameters)
  : AuxKernel(parameters),
    _j_integral_radius_inner(getParam<Real>("j_integral_radius_inner")),
    _j_integral_radius_outer(getParam<Real>("j_integral_radius_outer")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_point_index(isParamValid("crack_front_point_index")),
    _crack_front_point_index(
        _has_crack_front_point_index ? getParam<unsigned int>("crack_front_point_index") : 0),
    _treat_as_2d(false),
    _is_point_on_intersecting_boundary(false)
{
}

void
DomainIntegralQFunction::initialSetup()
{
  _treat_as_2d = _crack_front_definition->treatAs2D();
  bool using_mesh_cutter = _crack_front_definition->usingMeshCutter();
  if (_treat_as_2d && using_mesh_cutter == false)
  {
    if (_has_crack_front_point_index)
    {
      mooseWarning(
          "crack_front_point_index ignored because CrackFrontDefinition is set to treat as 2D");
    }
  }
  else
  {
    if (!_has_crack_front_point_index)
    {
      mooseError("crack_front_point_index must be specified in DomainIntegralQFunction");
    }
  }
  _is_point_on_intersecting_boundary =
      _crack_front_definition->isPointWithIndexOnIntersectingBoundary(_crack_front_point_index);
}

Real
DomainIntegralQFunction::computeValue()
{
  Real dist_to_crack_front;
  Real dist_along_tangent;
  projectToFrontAtPoint(dist_to_crack_front, dist_along_tangent);

  Real q = 1.0;
  if (dist_to_crack_front > _j_integral_radius_inner &&
      dist_to_crack_front < _j_integral_radius_outer)
    q = (_j_integral_radius_outer - dist_to_crack_front) /
        (_j_integral_radius_outer - _j_integral_radius_inner);
  else if (dist_to_crack_front >= _j_integral_radius_outer)
    q = 0.0;

  if (q > 0.0)
  {
    Real tangent_multiplier = 1.0;
    if (!_treat_as_2d)
    {
      const Real forward_segment_length =
          _crack_front_definition->getCrackFrontForwardSegmentLength(_crack_front_point_index);
      const Real backward_segment_length =
          _crack_front_definition->getCrackFrontBackwardSegmentLength(_crack_front_point_index);

      if (dist_along_tangent >= 0.0)
      {
        if (forward_segment_length > 0.0)
          tangent_multiplier = 1.0 - dist_along_tangent / forward_segment_length;
      }
      else
      {
        if (backward_segment_length > 0.0)
          tangent_multiplier = 1.0 + dist_along_tangent / backward_segment_length;
      }
    }

    tangent_multiplier = std::max(tangent_multiplier, 0.0);
    tangent_multiplier = std::min(tangent_multiplier, 1.0);

    // Set to zero if a node is on a designated free surface and its crack front node is not.
    if (_crack_front_definition->isNodeOnIntersectingBoundary(_current_node) &&
        !_is_point_on_intersecting_boundary)
      tangent_multiplier = 0.0;

    q *= tangent_multiplier;
  }

  return q;
}

void
DomainIntegralQFunction::projectToFrontAtPoint(Real & dist_to_front, Real & dist_along_tangent)
{
  const Point * crack_front_point =
      _crack_front_definition->getCrackFrontPoint(_crack_front_point_index);

  Point p = *_current_node;
  const RealVectorValue & crack_front_tangent =
      _crack_front_definition->getCrackFrontTangent(_crack_front_point_index);

  RealVectorValue crack_node_to_current_node = p - *crack_front_point;
  dist_along_tangent = crack_node_to_current_node * crack_front_tangent;
  RealVectorValue projection_point = *crack_front_point + dist_along_tangent * crack_front_tangent;
  RealVectorValue axis_to_current_node = p - projection_point;
  dist_to_front = axis_to_current_node.norm();
}
