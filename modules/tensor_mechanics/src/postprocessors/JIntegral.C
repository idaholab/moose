//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor calculates the J-Integral
//
#include "JIntegral.h"
#include "RankTwoTensor.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<JIntegral>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addClassDescription("Calculates the J-integral at a specified point "
                             " along the crack front");
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
  params.addParam<unsigned int>("ring_index", "Ring ID");
  params.addParam<unsigned int>("ring_first", "First Ring ID");
  MooseEnum q_function_type("Geometry Topology", "Geometry");
  params.addParam<MooseEnum>("q_function_type",
                             q_function_type,
                             "The method used to define the integration domain. Options are: " +
                                 q_function_type.getRawNames());
  return params;
}

JIntegral::JIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_point_index(isParamValid("crack_front_point_index")),
    _crack_front_point_index(
        _has_crack_front_point_index ? getParam<unsigned int>("crack_front_point_index") : 0),
    _treat_as_2d(false),
    _Eshelby_tensor(getMaterialProperty<RankTwoTensor>("Eshelby_tensor")),
    _J_thermal_term_vec(hasMaterialProperty<RealVectorValue>("J_thermal_term_vec")
                            ? &getMaterialProperty<RealVectorValue>("J_thermal_term_vec")
                            : NULL),
    _convert_J_to_K(getParam<bool>("convert_J_to_K")),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _poissons_ratio(isParamValid("poissons_ratio") ? getParam<Real>("poissons_ratio") : 0),
    _youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0),
    _ring_index(getParam<unsigned int>("ring_index")),
    _q_function_type(getParam<MooseEnum>("q_function_type"))
{
  if (_q_function_type == "TOPOLOGY")
    _ring_first = getParam<unsigned int>("ring_first");
}

void
JIntegral::initialSetup()
{
  _treat_as_2d = _crack_front_definition->treatAs2D();

  if (_convert_J_to_K && (!isParamValid("youngs_modulus") || !isParamValid("poissons_ratio")))
    mooseError("youngs_modulus and poissons_ratio must be specified if convert_J_to_K = true");
}

Real
JIntegral::computeQpIntegral()
{
  Real scalar_q = 0.0;
  RealVectorValue grad_of_scalar_q(0.0, 0.0, 0.0);

  const std::vector<std::vector<Real>> & phi_curr_elem = *_phi_curr_elem;
  const std::vector<std::vector<RealGradient>> & dphi_curr_elem = *_dphi_curr_elem;

  for (unsigned int i = 0; i < _current_elem->n_nodes(); ++i)
  {
    scalar_q += phi_curr_elem[i][_qp] * _q_curr_elem[i];

    for (unsigned int j = 0; j < _current_elem->dim(); ++j)
      grad_of_scalar_q(j) += dphi_curr_elem[i][_qp](j) * _q_curr_elem[i];
  }

  RankTwoTensor grad_of_vector_q;
  const RealVectorValue & crack_direction =
      _crack_front_definition->getCrackDirection(_crack_front_point_index);
  grad_of_vector_q(0, 0) = crack_direction(0) * grad_of_scalar_q(0);
  grad_of_vector_q(0, 1) = crack_direction(0) * grad_of_scalar_q(1);
  grad_of_vector_q(0, 2) = crack_direction(0) * grad_of_scalar_q(2);
  grad_of_vector_q(1, 0) = crack_direction(1) * grad_of_scalar_q(0);
  grad_of_vector_q(1, 1) = crack_direction(1) * grad_of_scalar_q(1);
  grad_of_vector_q(1, 2) = crack_direction(1) * grad_of_scalar_q(2);
  grad_of_vector_q(2, 0) = crack_direction(2) * grad_of_scalar_q(0);
  grad_of_vector_q(2, 1) = crack_direction(2) * grad_of_scalar_q(1);
  grad_of_vector_q(2, 2) = crack_direction(2) * grad_of_scalar_q(2);

  Real eq = _Eshelby_tensor[_qp].doubleContraction(grad_of_vector_q);

  // Thermal component
  Real eq_thermal = 0.0;
  if (_J_thermal_term_vec)
  {
    for (unsigned int i = 0; i < 3; i++)
      eq_thermal += crack_direction(i) * scalar_q * (*_J_thermal_term_vec)[_qp](i);
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
JIntegral::computeIntegral()
{
  Real sum = 0;

  // calculate phi and dphi for this element
  FEType fe_type(Utility::string_to_enum<Order>("first"),
                 Utility::string_to_enum<FEFamily>("lagrange"));
  const unsigned int dim = _current_elem->dim();
  std::unique_ptr<FEBase> fe(FEBase::build(dim, fe_type));
  fe->attach_quadrature_rule(_qrule);
  _phi_curr_elem = &fe->get_phi();
  _dphi_curr_elem = &fe->get_dphi();
  fe->reinit(_current_elem);

  // calculate q for all nodes in this element
  _q_curr_elem.clear();
  unsigned int ring_base = (_q_function_type == "TOPOLOGY") ? 0 : 1;

  for (unsigned int i = 0; i < _current_elem->n_nodes(); ++i)
  {
    Node * this_node = _current_elem->get_node(i);
    Real q_this_node;

    if (_q_function_type == "GEOMETRY")
      q_this_node = _crack_front_definition->DomainIntegralQFunction(
          _crack_front_point_index, _ring_index - ring_base, this_node);
    else if (_q_function_type == "TOPOLOGY")
      q_this_node = _crack_front_definition->DomainIntegralTopologicalQFunction(
          _crack_front_point_index, _ring_index - ring_base, this_node);

    _q_curr_elem.push_back(q_this_node);
  }

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
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
