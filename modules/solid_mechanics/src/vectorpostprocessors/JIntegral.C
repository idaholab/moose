//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JIntegral.h"
#include "RankTwoTensor.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "CrackFrontDefinition.h"
registerMooseObject("SolidMechanicsApp", JIntegral);

InputParameters
JIntegral::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  MooseEnum position_type("Angle Distance", "Distance");
  params.addParam<MooseEnum>(
      "position_type",
      position_type,
      "The method used to calculate position along crack front.  Options are: " +
          position_type.getRawNames());
  MooseEnum integral_vec("JIntegral CIntegral KFromJIntegral", "JIntegral");
  params.addRequiredParam<MooseEnum>("integral",
                                     integral_vec,
                                     "Domain integrals to calculate.  Choices are: " +
                                         integral_vec.getRawNames());
  params.addParam<unsigned int>("symmetry_plane",
                                "Account for a symmetry plane passing through "
                                "the plane of the crack, normal to the specified "
                                "axis (0=x, 1=y, 2=z)");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.set<bool>("use_displaced_mesh") = false;
  params.addRequiredParam<unsigned int>("ring_index", "Ring ID");
  MooseEnum q_function_type("Geometry Topology", "Geometry");
  params.addParam<MooseEnum>("q_function_type",
                             q_function_type,
                             "The method used to define the integration domain. Options are: " +
                                 q_function_type.getRawNames());
  params.addClassDescription("Computes the J-Integral, a measure of the strain energy release rate "
                             "at a crack tip, which can be used as a criterion for fracture "
                             "growth. It can, alternatively, compute the C(t) integral");
  return params;
}

JIntegral::JIntegral(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _integral(getParam<MooseEnum>("integral").getEnum<IntegralType>()),
    _J_thermal_term_vec(hasMaterialProperty<RealVectorValue>("J_thermal_term_vec")
                            ? &getMaterialProperty<RealVectorValue>("J_thermal_term_vec")
                            : nullptr),
    _Eshelby_tensor(_integral != IntegralType::CIntegral
                        ? &getMaterialProperty<RankTwoTensor>("Eshelby_tensor")
                        : nullptr),
    _Eshelby_tensor_dissipation(
        _integral == IntegralType::CIntegral
            ? &getMaterialProperty<RankTwoTensor>("Eshelby_tensor_dissipation")
            : nullptr),
    _fe_vars(getCoupledMooseVars()),
    _fe_type(_fe_vars[0]->feType()),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _poissons_ratio(isParamValid("poissons_ratio") ? getParam<Real>("poissons_ratio") : 0),
    _youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0),
    _ring_index(getParam<unsigned int>("ring_index")),
    _q_function_type(getParam<MooseEnum>("q_function_type").getEnum<QMethod>()),
    _position_type(getParam<MooseEnum>("position_type").getEnum<PositionType>()),
    _x(declareVector("x")),
    _y(declareVector("y")),
    _z(declareVector("z")),
    _position(declareVector("id")),
    _j_integral(declareVector((_integral == IntegralType::KFromJIntegral
                                   ? "K_"
                                   : (_integral == IntegralType::JIntegral ? "J_" : "C_")) +
                              Moose::stringify(_ring_index)))
{
}

void
JIntegral::initialSetup()
{
  _treat_as_2d = _crack_front_definition->treatAs2D();
  _using_mesh_cutter = _crack_front_definition->usingMeshCutter();
  if (_integral == IntegralType::KFromJIntegral &&
      (!isParamValid("youngs_modulus") || !isParamValid("poissons_ratio")))
    mooseError("youngs_modulus and poissons_ratio must be specified if integrals = KFromJIntegral");
}

void
JIntegral::initialize()
{
  std::size_t num_pts;
  if (_treat_as_2d && _using_mesh_cutter == false)
    num_pts = 1;
  else
    num_pts = _crack_front_definition->getNumCrackFrontPoints();

  _x.assign(num_pts, 0.0);
  _y.assign(num_pts, 0.0);
  _z.assign(num_pts, 0.0);
  _position.assign(num_pts, 0.0);
  _j_integral.assign(num_pts, 0.0);

  for (const auto * fe_var : _fe_vars)
  {
    if (fe_var->feType() != _fe_type)
      mooseError("displacements", "All coupled displacements must have the same type");
  }
}

Real
JIntegral::computeQpIntegral(const std::size_t crack_front_point_index,
                             const Real scalar_q,
                             const RealVectorValue & grad_of_scalar_q)
{
  RankTwoTensor grad_of_vector_q;
  const RealVectorValue & crack_direction =
      _crack_front_definition->getCrackDirection(crack_front_point_index);
  grad_of_vector_q(0, 0) = crack_direction(0) * grad_of_scalar_q(0);
  grad_of_vector_q(0, 1) = crack_direction(0) * grad_of_scalar_q(1);
  grad_of_vector_q(0, 2) = crack_direction(0) * grad_of_scalar_q(2);
  grad_of_vector_q(1, 0) = crack_direction(1) * grad_of_scalar_q(0);
  grad_of_vector_q(1, 1) = crack_direction(1) * grad_of_scalar_q(1);
  grad_of_vector_q(1, 2) = crack_direction(1) * grad_of_scalar_q(2);
  grad_of_vector_q(2, 0) = crack_direction(2) * grad_of_scalar_q(0);
  grad_of_vector_q(2, 1) = crack_direction(2) * grad_of_scalar_q(1);
  grad_of_vector_q(2, 2) = crack_direction(2) * grad_of_scalar_q(2);

  Real eq;

  if (_integral != IntegralType::CIntegral)
    eq = (*_Eshelby_tensor)[_qp].doubleContraction(grad_of_vector_q);
  else
    eq = (*_Eshelby_tensor_dissipation)[_qp].doubleContraction(grad_of_vector_q);

  // Thermal component
  Real eq_thermal = 0.0;
  if (_J_thermal_term_vec && _integral != IntegralType::CIntegral)
  {
    for (std::size_t i = 0; i < 3; i++)
      eq_thermal += crack_direction(i) * scalar_q * (*_J_thermal_term_vec)[_qp](i);
  }

  Real q_avg_seg = 1.0;
  if (!_crack_front_definition->treatAs2D())
  {
    q_avg_seg =
        (_crack_front_definition->getCrackFrontForwardSegmentLength(crack_front_point_index) +
         _crack_front_definition->getCrackFrontBackwardSegmentLength(crack_front_point_index)) /
        2.0;
  }

  Real etot = -eq + eq_thermal;

  return etot / q_avg_seg;
}

void
JIntegral::execute()
{
  // calculate phi and dphi for this element
  const std::size_t dim = _current_elem->dim();
  std::unique_ptr<libMesh::FEBase> fe(libMesh::FEBase::build(dim, _fe_type));
  fe->attach_quadrature_rule(const_cast<QBase *>(_qrule));
  _phi_curr_elem = &fe->get_phi();
  _dphi_curr_elem = &fe->get_dphi();
  fe->reinit(_current_elem);

  // calculate q for all nodes in this element
  std::size_t ring_base = (_q_function_type == QMethod::Topology) ? 0 : 1;

  for (std::size_t icfp = 0; icfp < _j_integral.size(); icfp++)
  {
    _q_curr_elem.clear();
    for (std::size_t i = 0; i < _current_elem->n_nodes(); ++i)
    {
      const Node * this_node = _current_elem->node_ptr(i);
      Real q_this_node;

      if (_q_function_type == QMethod::Geometry)
        q_this_node = _crack_front_definition->DomainIntegralQFunction(
            icfp, _ring_index - ring_base, this_node);
      else // QMethod::Topology
        q_this_node = _crack_front_definition->DomainIntegralTopologicalQFunction(
            icfp, _ring_index - ring_base, this_node);

      _q_curr_elem.push_back(q_this_node);
    }

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      Real scalar_q = 0.0;
      RealVectorValue grad_of_scalar_q(0.0, 0.0, 0.0);

      for (std::size_t i = 0; i < _current_elem->n_nodes(); ++i)
      {
        scalar_q += (*_phi_curr_elem)[i][_qp] * _q_curr_elem[i];

        for (std::size_t j = 0; j < _current_elem->dim(); ++j)
          grad_of_scalar_q(j) += (*_dphi_curr_elem)[i][_qp](j) * _q_curr_elem[i];
      }

      _j_integral[icfp] +=
          _JxW[_qp] * _coord[_qp] * computeQpIntegral(icfp, scalar_q, grad_of_scalar_q);
    }
  }
}

void
JIntegral::finalize()
{
  gatherSum(_j_integral);

  for (std::size_t i = 0; i < _j_integral.size(); ++i)
  {
    if (_has_symmetry_plane)
      _j_integral[i] *= 2.0;

    Real sign = (_j_integral[i] > 0.0) ? 1.0 : ((_j_integral[i] < 0.0) ? -1.0 : 0.0);

    if (_integral == IntegralType::KFromJIntegral)
      _j_integral[i] = sign * std::sqrt(std::abs(_j_integral[i]) * _youngs_modulus /
                                        (1.0 - Utility::pow<2>(_poissons_ratio)));

    const auto cfp = _crack_front_definition->getCrackFrontPoint(i);
    _x[i] = (*cfp)(0);
    _y[i] = (*cfp)(1);
    _z[i] = (*cfp)(2);

    if (_position_type == PositionType::Angle)
      _position[i] = _crack_front_definition->getAngleAlongFront(i);
    else
      _position[i] = _crack_front_definition->getDistanceAlongFront(i);
  }
}

void
JIntegral::threadJoin(const UserObject & y)
{
  const auto & uo = static_cast<const JIntegral &>(y);

  for (auto i = beginIndex(_j_integral); i < _j_integral.size(); ++i)
    _j_integral[i] += uo._j_integral[i];
}
