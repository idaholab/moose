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
  params.addParam<bool>("large", false, "Whether to include large or small Eshelby tensor");
  return params;
}

JIntegral::JIntegral(const std::string & name, InputParameters parameters) :
    ElementIntegralPostprocessor(name, parameters),
    _q(coupledValue("q")),
    _q_grad(coupledGradient("q")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_node_index(isParamValid("crack_front_node_index")),
    _crack_front_node_index(_has_crack_front_node_index ? getParam<unsigned int>("crack_front_node_index") : 0),
    _treat_as_2d(false),
    _Eshelby_tensor(getMaterialProperty<ColumnMajorMatrix>("Eshelby_tensor")),
    _Eshelby_tensor_small(getMaterialProperty<ColumnMajorMatrix>("Eshelby_tensor_small")),
    _large(getParam<bool>("large"))
    
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
  ColumnMajorMatrix gradient_of_q_grad;
  const RealVectorValue& crack_direction = _crack_front_definition->getCrackDirection(_crack_front_node_index);
  gradient_of_q_grad(0,0) = crack_direction(0)*_q_grad[_qp](0);//nodes... or integration points?
  gradient_of_q_grad(0,1) = crack_direction(0)*_q_grad[_qp](1);
  gradient_of_q_grad(0,2) = crack_direction(0)*_q_grad[_qp](2);
  gradient_of_q_grad(1,0) = crack_direction(1)*_q_grad[_qp](0);
  gradient_of_q_grad(1,1) = crack_direction(1)*_q_grad[_qp](1);
  gradient_of_q_grad(1,2) = crack_direction(1)*_q_grad[_qp](2);
  gradient_of_q_grad(2,0) = crack_direction(2)*_q_grad[_qp](0);
  gradient_of_q_grad(2,1) = crack_direction(2)*_q_grad[_qp](1);
  gradient_of_q_grad(2,2) = crack_direction(2)*_q_grad[_qp](2);

  Real eq = _Eshelby_tensor[_qp].doubleContraction(gradient_of_q_grad);
  
  Real eqs = _Eshelby_tensor_small[_qp].doubleContraction(gradient_of_q_grad);
  
  if(_large)
  {
    return eq;
  }
  else
  {
    return eqs;
  }
}
