//  This post processor returns the J-Integral
//
#include "JIntegral.h"

template<>
InputParameters validParams<JIntegral>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addCoupledVar("q", "The q function, aux variable");
  params.addRequiredParam<UserObjectName>("crack_front_definition","The CrackFrontDefinition user object name");
  params.addParam<unsigned int>("crack_front_node_index","The index of the node on the crack front corresponding to this q function");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

JIntegral::JIntegral(const std::string & name, InputParameters parameters) :
    ElementIntegralPostprocessor(name, parameters),
    _grad_of_scalar_q(coupledGradient("q")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_node_index(isParamValid("crack_front_node_index")),
    _crack_front_node_index(_has_crack_front_node_index ? getParam<unsigned int>("crack_front_node_index") : 0),
    _treat_as_2d(false),
    _Eshelby_tensor(getMaterialProperty<ColumnMajorMatrix>("Eshelby_tensor"))
{
}

void
JIntegral::initialSetup()
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
      mooseError("crack_front_node_index must be specified in qFunctionJIntegral3D");
    }
  }
}

Real
JIntegral::computeQpIntegral()
{
  ColumnMajorMatrix grad_of_vector_q;
  const RealVectorValue& crack_direction = _crack_front_definition->getCrackDirection(_crack_front_node_index);
  grad_of_vector_q(0,0) = crack_direction(0)*_grad_of_scalar_q[_qp](0);
  grad_of_vector_q(0,1) = crack_direction(0)*_grad_of_scalar_q[_qp](1);
  grad_of_vector_q(0,2) = crack_direction(0)*_grad_of_scalar_q[_qp](2);
  grad_of_vector_q(1,0) = crack_direction(1)*_grad_of_scalar_q[_qp](0);
  grad_of_vector_q(1,1) = crack_direction(1)*_grad_of_scalar_q[_qp](1);
  grad_of_vector_q(1,2) = crack_direction(1)*_grad_of_scalar_q[_qp](2);
  grad_of_vector_q(2,0) = crack_direction(2)*_grad_of_scalar_q[_qp](0);
  grad_of_vector_q(2,1) = crack_direction(2)*_grad_of_scalar_q[_qp](1);
  grad_of_vector_q(2,2) = crack_direction(2)*_grad_of_scalar_q[_qp](2);

  Real eq = _Eshelby_tensor[_qp].doubleContraction(grad_of_vector_q);

  Real q_avg_seg = 1.0;
  if (!_crack_front_definition->treatAs2D())
  {
    q_avg_seg = (_crack_front_definition->getCrackFrontForwardSegmentLength(_crack_front_node_index) +
                 _crack_front_definition->getCrackFrontBackwardSegmentLength(_crack_front_node_index)) / 2.0;
  }

  return -eq/q_avg_seg;
}
