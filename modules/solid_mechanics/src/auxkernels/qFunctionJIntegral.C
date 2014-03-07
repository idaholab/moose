#include "qFunctionJIntegral.h"

template<>
InputParameters validParams<qFunctionJIntegral>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredParam<Real>("j_integral_radius_inner", "Radius for J-Integral calculation");
  params.addRequiredParam<Real>("j_integral_radius_outer", "Radius for J-Integral calculation");
  params.addRequiredParam<UserObjectName>("crack_front_definition","The CrackFrontDefinition user object name");
  params.addParam<unsigned int>("crack_front_node_index","The index of the node on the crack front corresponding to this q function");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

qFunctionJIntegral::qFunctionJIntegral(const std::string & name, InputParameters parameters):
    AuxKernel(name, parameters),
    _j_integral_radius_inner(getParam<Real>("j_integral_radius_inner")),
    _j_integral_radius_outer(getParam<Real>("j_integral_radius_outer")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_node_index(isParamValid("crack_front_node_index")),
    _crack_front_node_index(_has_crack_front_node_index ? getParam<unsigned int>("crack_front_node_index") : 0),
    _treat_as_2d(false)
{}

void
qFunctionJIntegral::initialSetup()
{
  _treat_as_2d = _crack_front_definition->treatAs2D();

  if (_treat_as_2d)
  {
    if (_has_crack_front_node_index)
    {
      mooseWarning("crack_front_node_index ignored because CrackFrontDefinition is set to treat as 2D");
    }
  }
  else
  {
    if (!_has_crack_front_node_index)
    {
      mooseError("crack_front_node_index must be specified in qFunctionJIntegral");
    }
  }
}

Real
qFunctionJIntegral::computeValue()
{
  Point  p = *_current_node;

  const Node &crack_front_node = _crack_front_definition->getCrackFrontNode(_crack_front_node_index);
  const RealVectorValue &crack_front_tangent = _crack_front_definition->getCrackFrontTangent(_crack_front_node_index);

  RealVectorValue crack_node_to_current_node = p - crack_front_node;
  Real dist_along_tangent = crack_node_to_current_node * crack_front_tangent;
  RealVectorValue projection_point = crack_front_node + dist_along_tangent * crack_front_tangent;
  RealVectorValue axis_to_current_node = p - projection_point;
  Real dist_to_crack_front = axis_to_current_node.size();

  Real q = 1.0;
  if ( dist_to_crack_front > _j_integral_radius_inner &&
       dist_to_crack_front < _j_integral_radius_outer)
  {
    q = (_j_integral_radius_outer - dist_to_crack_front) /
        (_j_integral_radius_outer - _j_integral_radius_inner);
  }
  else if ( dist_to_crack_front >= _j_integral_radius_outer)
  {
    q = 0.0;
  }

  if ((q > 0.0) && (!_treat_as_2d))
  {
    const Real forward_segment_length = _crack_front_definition->getCrackFrontForwardSegmentLength(_crack_front_node_index);
    const Real backward_segment_length = _crack_front_definition->getCrackFrontBackwardSegmentLength(_crack_front_node_index);

    if (dist_along_tangent >= 0.0)
    {
      if (dist_along_tangent >= forward_segment_length)
      {
        if (forward_segment_length > 0.0)
        {
          q = 0.0;
        }
      }
      else
      {
        q *= (1.0 - dist_along_tangent/forward_segment_length);
      }
    }
    else
    {
      if (-dist_along_tangent >= backward_segment_length)
      {
        if (backward_segment_length > 0.0)
        {
          q = 0.0;
        }
      }
      else
      {
        q *= (1.0 + dist_along_tangent/backward_segment_length);
      }
    }
  }

  return q;
}
