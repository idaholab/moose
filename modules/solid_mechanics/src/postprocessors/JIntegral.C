/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
//  This post processor calculates the J-Integral
//
#include "JIntegral.h"

template <>
InputParameters
validParams<JIntegral>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addCoupledVar("q", "The q function, aux variable");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addParam<unsigned int>(
      "crack_front_point_index",
      "The index of the point on the crack front corresponding to this q function");
  params.addParam<bool>(
      "convert_J_to_K", false, "Convert J-integral to stress intensity factor K.");
  params.addParam<unsigned int>("symmetry_plane",
                                "Account for a symmetry plane passing through "
                                "the plane of the crack, normal to the specified "
                                "axis (0=x, 1=y, 2=z)");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

JIntegral::JIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _scalar_q(coupledValue("q")),
    _grad_of_scalar_q(coupledGradient("q")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_point_index(isParamValid("crack_front_point_index")),
    _crack_front_point_index(
        _has_crack_front_point_index ? getParam<unsigned int>("crack_front_point_index") : 0),
    _treat_as_2d(false),
    _Eshelby_tensor(getMaterialProperty<ColumnMajorMatrix>("Eshelby_tensor")),
    _J_thermal_term_vec(hasMaterialProperty<RealVectorValue>("J_thermal_term_vec")
                            ? &getMaterialProperty<RealVectorValue>("J_thermal_term_vec")
                            : NULL),
    _convert_J_to_K(getParam<bool>("convert_J_to_K")),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _poissons_ratio(isParamValid("poissons_ratio") ? getParam<Real>("poissons_ratio") : 0),
    _youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0)
{
}

void
JIntegral::initialSetup()
{
  _treat_as_2d = _crack_front_definition->treatAs2D();

  if (_treat_as_2d)
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
      mooseError("crack_front_point_index must be specified in JIntegral");
    }
  }

  if (_convert_J_to_K && (!isParamValid("youngs_modulus") || !isParamValid("poissons_ratio")))
    mooseError("youngs_modulus and poissons_ratio must be specified if convert_J_to_K = true");
}

Real
JIntegral::computeQpIntegral()
{
  ColumnMajorMatrix grad_of_vector_q;
  const RealVectorValue & crack_direction =
      _crack_front_definition->getCrackDirection(_crack_front_point_index);
  grad_of_vector_q(0, 0) = crack_direction(0) * _grad_of_scalar_q[_qp](0);
  grad_of_vector_q(0, 1) = crack_direction(0) * _grad_of_scalar_q[_qp](1);
  grad_of_vector_q(0, 2) = crack_direction(0) * _grad_of_scalar_q[_qp](2);
  grad_of_vector_q(1, 0) = crack_direction(1) * _grad_of_scalar_q[_qp](0);
  grad_of_vector_q(1, 1) = crack_direction(1) * _grad_of_scalar_q[_qp](1);
  grad_of_vector_q(1, 2) = crack_direction(1) * _grad_of_scalar_q[_qp](2);
  grad_of_vector_q(2, 0) = crack_direction(2) * _grad_of_scalar_q[_qp](0);
  grad_of_vector_q(2, 1) = crack_direction(2) * _grad_of_scalar_q[_qp](1);
  grad_of_vector_q(2, 2) = crack_direction(2) * _grad_of_scalar_q[_qp](2);

  Real eq = _Eshelby_tensor[_qp].doubleContraction(grad_of_vector_q);

  // Thermal component
  Real eq_thermal = 0.0;
  if (_J_thermal_term_vec)
  {
    for (unsigned int i = 0; i < 3; i++)
      eq_thermal += crack_direction(i) * _scalar_q[_qp] * (*_J_thermal_term_vec)[_qp](i);
  }

  Real q_avg_seg = 1.0;
  if (!_crack_front_definition->treatAs2D())
  {
    q_avg_seg =
        (_crack_front_definition->getCrackFrontForwardSegmentLength(_crack_front_point_index) +
         _crack_front_definition->getCrackFrontBackwardSegmentLength(_crack_front_point_index)) /
        2.0;
  }

  Real etot = -eq + eq_thermal;

  return etot / q_avg_seg;
}

Real
JIntegral::getValue()
{
  gatherSum(_integral_value);
  if (_has_symmetry_plane)
    _integral_value *= 2.0;

  Real sign = (_integral_value > 0.0) ? 1.0 : ((_integral_value < 0.0) ? -1.0 : 0.0);
  if (_convert_J_to_K)
    _integral_value = sign * std::sqrt(std::abs(_integral_value) * _youngs_modulus /
                                       (1 - std::pow(_poissons_ratio, 2)));

  return _integral_value;
}
