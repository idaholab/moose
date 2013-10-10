//  This post processor returns the J-Integral
//
#include "JIntegral.h"

template<>
InputParameters validParams<JIntegral>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addCoupledVar("q", "The q function, aux variable");
  params.addRequiredParam<RealVectorValue>("direction","Direction of the q function");
  params.addParam<bool>("large", false, "Whether to include large or small Eshelpy tensor");
  return params;
}

JIntegral::JIntegral(const std::string & name, InputParameters parameters) :
    ElementIntegralPostprocessor(name, parameters),
    _q(coupledValue("q")),
    _q_grad(coupledGradient("q")),
    _direction(getParam<RealVectorValue>("direction")),
    _Eshelby_tensor(getMaterialProperty<ColumnMajorMatrix>("Eshelby_tensor")),
    _Eshelby_tensor_small(getMaterialProperty<ColumnMajorMatrix>("Eshelby_tensor_small")),
    _large(getParam<bool>("large"))
    
{
}

Real
JIntegral::computeQpIntegral()
{
  ColumnMajorMatrix gradient_of_q_grad;
  gradient_of_q_grad(0,0) = _direction(0)*_q_grad[_qp](0);//nodes... or integration points?
  gradient_of_q_grad(0,1) = _direction(0)*_q_grad[_qp](1);
  gradient_of_q_grad(0,2) = _direction(0)*_q_grad[_qp](2);
  gradient_of_q_grad(1,0) = _direction(1)*_q_grad[_qp](0);
  gradient_of_q_grad(1,1) = _direction(1)*_q_grad[_qp](1);
  gradient_of_q_grad(1,2) = _direction(1)*_q_grad[_qp](2);
  gradient_of_q_grad(2,0) = _direction(2)*_q_grad[_qp](0);
  gradient_of_q_grad(2,1) = _direction(2)*_q_grad[_qp](1);
  gradient_of_q_grad(2,2) = _direction(2)*_q_grad[_qp](2);

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
