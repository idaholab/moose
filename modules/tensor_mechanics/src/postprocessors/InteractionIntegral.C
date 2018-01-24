//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the Interaction Integral
//
#include "InteractionIntegral.h"
#include "MooseMesh.h"
#include "RankTwoTensor.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature.h"
#include "DerivativeMaterialInterface.h"
#include "libmesh/utility.h"

MooseEnum
InteractionIntegral::qFunctionType()
{
  return MooseEnum("Geometry Topology", "Geometry");
}

MooseEnum
InteractionIntegral::sifModeType()
{
  return MooseEnum("KI KII KIII T", "KI");
}

template <>
InputParameters
validParams<InteractionIntegral>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addCoupledVar("temp",
                       "The temperature (optional). Must be provided to correctly compute "
                       "stress intensity factors in models with thermal strain gradients.");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addParam<unsigned int>(
      "crack_front_point_index",
      "The index of the point on the crack front corresponding to this q function");
  params.addParam<Real>(
      "K_factor", "Conversion factor between interaction integral and stress intensity factor K");
  params.addParam<unsigned int>("symmetry_plane",
                                "Account for a symmetry plane passing through "
                                "the plane of the crack, normal to the specified "
                                "axis (0=x, 1=y, 2=z)");
  params.addParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.addParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.set<bool>("use_displaced_mesh") = false;
  params.addParam<unsigned int>("ring_index", "Ring ID");
  params.addParam<MooseEnum>("q_function_type",
                             InteractionIntegral::qFunctionType(),
                             "The method used to define the integration domain. Options are: " +
                                 InteractionIntegral::qFunctionType().getRawNames());
  params.addRequiredParam<MooseEnum>("sif_mode",
                                     InteractionIntegral::sifModeType(),
                                     "Stress intensity factor to calculate. Choices are: " +
                                         InteractionIntegral::sifModeType().getRawNames());

  return params;
}

InteractionIntegral::InteractionIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _ndisp(coupledComponents("displacements")),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _has_crack_front_point_index(isParamValid("crack_front_point_index")),
    _crack_front_point_index(
        _has_crack_front_point_index ? getParam<unsigned int>("crack_front_point_index") : 0),
    _treat_as_2d(false),
    _stress(hasMaterialProperty<RankTwoTensor>("stress")
                ? &getMaterialPropertyByName<RankTwoTensor>("stress")
                : nullptr),
    _strain(hasMaterialProperty<RankTwoTensor>("elastic_strain")
                ? &getMaterialPropertyByName<RankTwoTensor>("elastic_strain")
                : nullptr),
    _grad_disp(3),
    _has_temp(isCoupled("temp")),
    _grad_temp(_has_temp ? coupledGradient("temp") : _grad_zero),
    _K_factor(getParam<Real>("K_factor")),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _youngs_modulus(getParam<Real>("youngs_modulus")),
    _ring_index(getParam<unsigned int>("ring_index")),
    _total_deigenstrain_dT(hasMaterialProperty<RankTwoTensor>("total_deigenstrain_dT")
                               ? &getMaterialProperty<RankTwoTensor>("total_deigenstrain_dT")
                               : nullptr),
    _q_function_type(getParam<MooseEnum>("q_function_type").getEnum<QMethod>()),
    _sif_mode(getParam<MooseEnum>("sif_mode").getEnum<SifMethod>())
{
  if (!hasMaterialProperty<RankTwoTensor>("stress"))
    mooseError("InteractionIntegral Error: RankTwoTensor material property 'stress' not found. "
               "This may be because solid mechanics system is being used to calculate a SymmTensor "
               "'stress' material property. To use interaction integral calculation with solid "
               "mechanics application, please set 'solid_mechanics = true' in the DomainIntegral "
               "block.");

  if (!hasMaterialProperty<RankTwoTensor>("elastic_strain"))
    mooseError("InteractionIntegral Error: RankTwoTensor material property 'elastic_strain' not "
               "found. This may be because solid mechanics system is being used to calculate a "
               "SymmTensor 'elastic_strain' material property. To use interaction integral "
               "calculation with solid mechanics application, please set 'solid_mechanics = true' "
               "in the DomainIntegral block.");

  if (_has_temp && !_total_deigenstrain_dT)
    mooseError("InteractionIntegral Error: To include thermal strain term in interaction integral, "
               "must both couple temperature in DomainIntegral block and compute "
               "total_deigenstrain_dT using ThermalFractureIntegral material model.");

  // plane strain
  _kappa = 3.0 - 4.0 * _poissons_ratio;
  _shear_modulus = _youngs_modulus / (2.0 * (1.0 + _poissons_ratio));

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("InteractionIntegral Error: number of variables supplied in 'displacements' must "
               "match the mesh dimension.");

  // fetch gradients of coupled variables
  for (unsigned int i = 0; i < _ndisp; ++i)
    _grad_disp[i] = &coupledGradient("displacements", i);

  // set unused dimensions to zero
  for (unsigned i = _ndisp; i < 3; ++i)
    _grad_disp[i] = &_grad_zero;
}

void
InteractionIntegral::initialSetup()
{
  _treat_as_2d = _crack_front_definition->treatAs2D();
}

Real
InteractionIntegral::getValue()
{
  gatherSum(_integral_value);

  if (_sif_mode == SifMethod::T && !_treat_as_2d)
    _integral_value +=
        _poissons_ratio *
        _crack_front_definition->getCrackFrontTangentialStrain(_crack_front_point_index);

  return _K_factor * _integral_value;
}

Real
InteractionIntegral::computeQpIntegral()
{
  Real scalar_q = 0.0;
  RealVectorValue grad_q(0.0, 0.0, 0.0);

  const std::vector<std::vector<Real>> & phi_curr_elem = *_phi_curr_elem;
  const std::vector<std::vector<RealGradient>> & dphi_curr_elem = *_dphi_curr_elem;

  for (unsigned int i = 0; i < _current_elem->n_nodes(); ++i)
  {
    scalar_q += phi_curr_elem[i][_qp] * _q_curr_elem[i];

    for (unsigned int j = 0; j < _current_elem->dim(); ++j)
      grad_q(j) += dphi_curr_elem[i][_qp](j) * _q_curr_elem[i];
  }

  // In the crack front coordinate system, the crack direction is (1,0,0)
  RealVectorValue crack_direction(0.0);
  crack_direction(0) = 1.0;

  // Calculate (r,theta) position of qp relative to crack front
  Point p(_q_point[_qp]);
  _crack_front_definition->calculateRThetaToCrackFront(p, _crack_front_point_index, _r, _theta);

  RankTwoTensor aux_stress;
  RankTwoTensor aux_du;

  if (_sif_mode == SifMethod::KI || _sif_mode == SifMethod::KII || _sif_mode == SifMethod::KIII)
    computeAuxFields(aux_stress, aux_du);
  else if (_sif_mode == SifMethod::T)
    computeTFields(aux_stress, aux_du);

  RankTwoTensor grad_disp((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

  // Rotate stress, strain, displacement and temperature to crack front coordinate system
  RealVectorValue grad_q_cf =
      _crack_front_definition->rotateToCrackFrontCoords(grad_q, _crack_front_point_index);
  RankTwoTensor grad_disp_cf =
      _crack_front_definition->rotateToCrackFrontCoords(grad_disp, _crack_front_point_index);
  RankTwoTensor stress_cf =
      _crack_front_definition->rotateToCrackFrontCoords((*_stress)[_qp], _crack_front_point_index);
  RankTwoTensor strain_cf =
      _crack_front_definition->rotateToCrackFrontCoords((*_strain)[_qp], _crack_front_point_index);
  RealVectorValue grad_temp_cf =
      _crack_front_definition->rotateToCrackFrontCoords(_grad_temp[_qp], _crack_front_point_index);

  RankTwoTensor dq;
  dq(0, 0) = crack_direction(0) * grad_q_cf(0);
  dq(0, 1) = crack_direction(0) * grad_q_cf(1);
  dq(0, 2) = crack_direction(0) * grad_q_cf(2);

  // Calculate interaction integral terms

  // Term1 = stress * x1-derivative of aux disp * dq
  RankTwoTensor tmp1 = dq * stress_cf;
  Real term1 = aux_du.doubleContraction(tmp1);

  // Term2 = aux stress * x1-derivative of disp * dq
  RankTwoTensor tmp2 = dq * aux_stress;
  Real term2 = grad_disp_cf(0, 0) * tmp2(0, 0) + grad_disp_cf(1, 0) * tmp2(0, 1) +
               grad_disp_cf(2, 0) * tmp2(0, 2);

  // Term3 = aux stress * strain * dq_x   (= stress * aux strain * dq_x)
  Real term3 = dq(0, 0) * aux_stress.doubleContraction(strain_cf);

  // Term4 (thermal strain term) = q * aux_stress * alpha * dtheta_x
  // - the term including the derivative of alpha is not implemented
  Real term4 = 0.0;
  if (_has_temp)
  {
    Real sigma_alpha = aux_stress.doubleContraction((*_total_deigenstrain_dT)[_qp]);
    term4 = scalar_q * sigma_alpha * grad_temp_cf(0);
  }

  Real q_avg_seg = 1.0;
  if (!_crack_front_definition->treatAs2D())
  {
    q_avg_seg =
        (_crack_front_definition->getCrackFrontForwardSegmentLength(_crack_front_point_index) +
         _crack_front_definition->getCrackFrontBackwardSegmentLength(_crack_front_point_index)) /
        2.0;
  }

  Real eq = term1 + term2 - term3 + term4;

  if (_has_symmetry_plane)
    eq *= 2.0;

  return eq / q_avg_seg;
}

Real
InteractionIntegral::computeIntegral()
{
  Real sum = 0.0;

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
  unsigned int ring_base = (_q_function_type == QMethod::Topology) ? 0 : 1;

  for (unsigned int i = 0; i < _current_elem->n_nodes(); ++i)
  {
    Node * this_node = _current_elem->get_node(i);
    Real q_this_node;

    if (_q_function_type == QMethod::Geometry)
      q_this_node = _crack_front_definition->DomainIntegralQFunction(
          _crack_front_point_index, _ring_index - ring_base, this_node);
    else if (_q_function_type == QMethod::Topology)
      q_this_node = _crack_front_definition->DomainIntegralTopologicalQFunction(
          _crack_front_point_index, _ring_index - ring_base, this_node);

    _q_curr_elem.push_back(q_this_node);
  }

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
}

void
InteractionIntegral::computeAuxFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp)
{
  RealVectorValue k(0.0);
  if (_sif_mode == SifMethod::KI)
    k(0) = 1.0;
  else if (_sif_mode == SifMethod::KII)
    k(1) = 1.0;
  else if (_sif_mode == SifMethod::KIII)
    k(2) = 1.0;

  Real t = _theta;
  Real t2 = _theta / 2.0;
  Real tt2 = 3.0 * _theta / 2.0;
  Real st = std::sin(t);
  Real ct = std::cos(t);
  Real st2 = std::sin(t2);
  Real ct2 = std::cos(t2);
  Real stt2 = std::sin(tt2);
  Real ctt2 = std::cos(tt2);
  Real ct2sq = Utility::pow<2>(ct2);
  Real ct2cu = Utility::pow<3>(ct2);
  Real sqrt2PiR = std::sqrt(2.0 * libMesh::pi * _r);

  // Calculate auxiliary stress tensor
  aux_stress.zero();

  aux_stress(0, 0) =
      1.0 / sqrt2PiR * (k(0) * ct2 * (1.0 - st2 * stt2) - k(1) * st2 * (2.0 + ct2 * ctt2));
  aux_stress(1, 1) = 1.0 / sqrt2PiR * (k(0) * ct2 * (1.0 + st2 * stt2) + k(1) * st2 * ct2 * ctt2);
  aux_stress(0, 1) = 1.0 / sqrt2PiR * (k(0) * ct2 * st2 * ctt2 + k(1) * ct2 * (1.0 - st2 * stt2));
  aux_stress(0, 2) = -1.0 / sqrt2PiR * k(2) * st2;
  aux_stress(1, 2) = 1.0 / sqrt2PiR * k(2) * ct2;
  // plane stress
  // Real s33 = 0;
  // plane strain
  aux_stress(2, 2) = _poissons_ratio * (aux_stress(0, 0) + aux_stress(1, 1));

  aux_stress(1, 0) = aux_stress(0, 1);
  aux_stress(2, 0) = aux_stress(0, 2);
  aux_stress(2, 1) = aux_stress(1, 2);

  // Calculate x1 derivative of auxiliary displacements
  grad_disp.zero();

  grad_disp(0, 0) = k(0) / (4.0 * _shear_modulus * sqrt2PiR) *
                        (ct * ct2 * _kappa + ct * ct2 - 2.0 * ct * ct2cu + st * st2 * _kappa +
                         st * st2 - 6.0 * st * st2 * ct2sq) +
                    k(1) / (4.0 * _shear_modulus * sqrt2PiR) *
                        (ct * st2 * _kappa + ct * st2 + 2.0 * ct * st2 * ct2sq - st * ct2 * _kappa +
                         3.0 * st * ct2 - 6.0 * st * ct2cu);

  grad_disp(0, 1) = k(0) / (4.0 * _shear_modulus * sqrt2PiR) *
                        (ct * st2 * _kappa + ct * st2 - 2.0 * ct * st2 * ct2sq - st * ct2 * _kappa -
                         5.0 * st * ct2 + 6.0 * st * ct2cu) +
                    k(1) / (4.0 * _shear_modulus * sqrt2PiR) *
                        (-ct * ct2 * _kappa + 3.0 * ct * ct2 - 2.0 * ct * ct2cu -
                         st * st2 * _kappa + 3.0 * st * st2 - 6.0 * st * st2 * ct2sq);

  grad_disp(0, 2) = k(2) / (_shear_modulus * sqrt2PiR) * (st2 * ct - ct2 * st);
}

void
InteractionIntegral::computeTFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp)
{

  Real t = _theta;
  Real st = std::sin(t);
  Real ct = std::cos(t);
  Real stsq = Utility::pow<2>(st);
  Real ctsq = Utility::pow<2>(ct);
  Real ctcu = Utility::pow<3>(ct);
  Real oneOverPiR = 1.0 / (libMesh::pi * _r);

  aux_stress.zero();
  aux_stress(0, 0) = -oneOverPiR * ctcu;
  aux_stress(0, 1) = -oneOverPiR * st * ctsq;
  aux_stress(1, 0) = -oneOverPiR * st * ctsq;
  aux_stress(1, 1) = -oneOverPiR * ct * stsq;
  aux_stress(2, 2) = -oneOverPiR * _poissons_ratio * (ctcu + ct * stsq);

  grad_disp.zero();
  grad_disp(0, 0) = oneOverPiR / (4.0 * _youngs_modulus) *
                    (ct * (4.0 * Utility::pow<2>(_poissons_ratio) - 3.0 + _poissons_ratio) -
                     std::cos(3.0 * t) * (1.0 + _poissons_ratio));
  grad_disp(0, 1) = -oneOverPiR / (4.0 * _youngs_modulus) *
                    (st * (4.0 * Utility::pow<2>(_poissons_ratio) - 3.0 + _poissons_ratio) +
                     std::sin(3.0 * t) * (1.0 + _poissons_ratio));
}
