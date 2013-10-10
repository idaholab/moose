#include "qFunctionJIntegral.h"

template<>
InputParameters validParams<qFunctionJIntegral>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredParam<Real>("j_integral_radius_inner", "Radius for J-Integral calculation");
  params.addRequiredParam<Real>("j_integral_radius_outer", "Radius for J-Integral calculation");
  params.addRequiredParam<RealVectorValue>("crack_location","The location of the crack point");
  return params;
}

qFunctionJIntegral::qFunctionJIntegral(const std::string & name, InputParameters parameters):
    AuxKernel(name, parameters),
   _j_integral_radius_inner(getParam<Real>("j_integral_radius_inner")),
    _j_integral_radius_outer(getParam<Real>("j_integral_radius_outer")),
    _crack_location(getParam<RealVectorValue>("crack_location"))
   
{}

Real
qFunctionJIntegral::computeValue()
{
  Point  p = *_current_node;
  Real p_length = std::sqrt((p(0)-_crack_location(0))*(p(0)-_crack_location(0)) + (p(1)-_crack_location(1))*(p(1)-_crack_location(1)));
  
//  std::cout << p(0) << std::endl;
//  std::cout << p(1) << std::endl;
//  std::cout << p(2) << std::endl;
//  std::cout << "p_length = " << p_length << std::endl;
  
  if ( p_length <= _j_integral_radius_inner)
    return 1;
  if ( p_length > _j_integral_radius_inner && p_length < _j_integral_radius_outer)
    return (_j_integral_radius_outer - p_length) / (_j_integral_radius_outer - _j_integral_radius_inner);
  if ( p_length >= _j_integral_radius_outer)
    return 0.0;
  else
    return 0.0;
}
