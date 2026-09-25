//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCZMInterfaceKernelSmallStrain.h"
#include "ElasticityTensorTools.h"
#include "StabilizationUtils.h"

namespace
{
RankTwoTensor
gradOpForCoord(const Moose::CoordinateSystemType coord_sys,
               const unsigned int component,
               const RealVectorValue & grad,
               const Real value,
               const Point & point)
{
  RankTwoTensor grad_op;

  switch (coord_sys)
  {
    case Moose::COORD_XYZ:
      for (const auto j : make_range(3))
        grad_op(component, j) = grad(j);
      break;

    case Moose::COORD_RZ:
      for (const auto j : make_range(2))
        grad_op(component, j) = grad(j);

      if (component == 0)
        grad_op(2, 2) = value / point(0);
      break;

    case Moose::COORD_RSPHERICAL:
      grad_op(0, 0) = grad(0);
      grad_op(1, 1) = value / point(0);
      grad_op(2, 2) = value / point(0);
      break;

    default:
      mooseError("Unsupported coordinate system for SCZMInterfaceKernelSmallStrain.");
  }

  return grad_op;
}
}

registerMooseObject("ShiftedBoundaryMethodApp", SCZMInterfaceKernelSmallStrain);
registerMooseObject("ShiftedBoundaryMethodApp", ADSCZMInterfaceKernelSmallStrain);

template <bool is_ad>
InputParameters
SCZMInterfaceKernelSmallStrainTempl<is_ad>::validParams()
{
  InputParameters params = SCZMInterfaceKernelBaseTempl<is_ad>::validParams();

  params.addParam<bool>("directional_correction", true, "Add the directional correction terms.");
  params.addParam<MaterialPropertyName>(
      "stress", "stress", "Name of the stress tensor material property.");
  if constexpr (!is_ad)
  {
    params.addParam<MaterialPropertyName>(
        "tangent", "Jacobian_mult", "Name of the material Jacobian tensor property.");
    params.addParam<MooseEnum>(
        "tangent_definition",
        MooseEnum("auto stress_wrt_strain pk1_wrt_deformation_gradient", "auto"),
        "Mathematical definition of the tangent material property. 'auto' recognizes the standard "
        "Jacobian_mult property as d(stress)/d(strain) and pk1_jacobian as d(PK1)/d(F). Select "
        "'stress_wrt_strain' or 'pk1_wrt_deformation_gradient' explicitly for a custom tangent "
        "property; use the latter only when the property is known to be d(PK1)/d(F).");
    params.addParam<bool>(
        "volumetric_locking_correction",
        false,
        "Whether to apply volumetric locking correction to the directional "
        "correction Jacobian. This setting should be consistent with the volumetric "
        "locking correction used in the bulk finite element formulation to ensure "
        "the interface Jacobian is consistent with the bulk stress.");
  }
  params.addClassDescription(
      "Shifted CZM Interface kernel to use when using the Small Strain kinematic formulation.");

  return params;
}

template <bool is_ad>
SCZMInterfaceKernelSmallStrainTempl<is_ad>::SCZMInterfaceKernelSmallStrainTempl(
    const InputParameters & parameters)
  : SCZMInterfaceKernelBaseTempl<is_ad>(parameters),
    _stress(this->template getGenericMaterialPropertyByName<RankTwoTensor, is_ad>(
        this->_base_name + parameters.get<MaterialPropertyName>("stress"))),
    _stress_neighbor(this->template getGenericNeighborMaterialPropertyByName<RankTwoTensor, is_ad>(
        this->_base_name + parameters.get<MaterialPropertyName>("stress"))),
    _dtraction_djump_global(nullptr),
    _Jacobian_mult(nullptr),
    _Jacobian_mult_neighbor(nullptr),
    _tangent_is_dpk1_df(false),
    _directional_correction(parameters.get<bool>("directional_correction")),
    _volumetric_locking_correction(false)
{
  if constexpr (!is_ad)
  {
    _disp_var.resize(this->_ndisp);
    _vars.resize(this->_ndisp);
    for (const auto i : make_range(this->_ndisp))
    {
      _disp_var[i] = this->coupled("displacements", i);
      _vars[i] = this->getVar("displacements", i);
    }

    _dtraction_djump_global = &this->template getMaterialPropertyByName<RankTwoTensor>(
        this->_base_name + "dtraction_djump_global");
    _Jacobian_mult = &this->template getMaterialPropertyByName<RankFourTensor>(
        this->_base_name + parameters.get<MaterialPropertyName>("tangent"));
    _Jacobian_mult_neighbor = &this->template getNeighborMaterialPropertyByName<RankFourTensor>(
        this->_base_name + parameters.get<MaterialPropertyName>("tangent"));
    _volumetric_locking_correction = parameters.get<bool>("volumetric_locking_correction");

    const auto & tangent_name = parameters.get<MaterialPropertyName>("tangent");
    const auto & tangent_definition = parameters.get<MooseEnum>("tangent_definition");
    if (tangent_definition == "auto")
    {
      if (tangent_name == "Jacobian_mult")
        _tangent_is_dpk1_df = false;
      else if (tangent_name == "pk1_jacobian")
        _tangent_is_dpk1_df = true;
      else
        this->paramError("tangent_definition",
                         "Cannot infer the mathematical definition of custom tangent property '",
                         tangent_name,
                         "'. Set 'tangent_definition' to 'stress_wrt_strain' or "
                         "'pk1_wrt_deformation_gradient'.");
    }
    else
      _tangent_is_dpk1_df = tangent_definition == "pk1_wrt_deformation_gradient";
  }
}

template <bool is_ad>
GenericReal<is_ad>
SCZMInterfaceKernelSmallStrainTempl<is_ad>::computeQpResidual(Moose::DGResidualType type)
{
  if (!this->_shifted)
    return SCZMInterfaceKernelBaseTempl<is_ad>::computeQpResidual(type);

  const RealVectorValue true_normal(this->trueNormal());
  const Real true_normal_dot_surrogate_normal = true_normal * this->_normals[this->_qp];

  auto residual = SCZMInterfaceKernelBaseTempl<is_ad>::computeQpResidual(type);
  residual *= true_normal_dot_surrogate_normal;

  if (_directional_correction)
  {
    const auto stress = _stress[this->_qp].row(this->_component);
    const auto stress_neighbor = _stress_neighbor[this->_qp].row(this->_component);
    const auto normal_tangent =
        this->_normals[this->_qp] - true_normal_dot_surrogate_normal * true_normal;

    switch (type)
    {
      case Moose::Element:
        residual -= this->_test[this->_i][this->_qp] * (stress * normal_tangent);
        break;

      case Moose::Neighbor:
        residual += this->_test_neighbor[this->_i][this->_qp] * (stress_neighbor * normal_tangent);
        break;

      default:
        break;
    }
  }

  return residual;
}

template <bool is_ad>
Real
SCZMInterfaceKernelSmallStrainTempl<is_ad>::computeDResidualDDisplacement(
    unsigned int component_j, Moose::DGJacobianType type) const
{
  if constexpr (is_ad)
    return 0;
  else
    return computeDResidualDDisplacement(component_j, type);
}

template <>
Real
SCZMInterfaceKernelSmallStrainTempl<false>::computeDResidualDDisplacement(
    unsigned int component_j, Moose::DGJacobianType type) const
{
  const auto jacsd = (*_dtraction_djump_global)[this->_qp](this->_component, component_j);
  Real jac = jacsd;

  if (!_shifted)
  {
    switch (type)
    {
      case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
        jac *= _test[_i][_qp] * _vars[component_j]->phiFace()[_j][_qp];
        break;
      case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
        jac *= -_test[_i][_qp] * _vars[component_j]->phiFaceNeighbor()[_j][_qp];
        break;
      case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
        jac *= -_test_neighbor[_i][_qp] * _vars[component_j]->phiFace()[_j][_qp];
        break;
      case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
        jac *= _test_neighbor[_i][_qp] * _vars[component_j]->phiFaceNeighbor()[_j][_qp];
        break;
    }
    return jac;
  }

  const auto d = surrogateDistance();
  const auto true_normal = RealVectorValue(trueNormal());
  const auto true_normal_dot_surrogate_normal = true_normal * _normals[_qp];

  // Shifted derivative uses d(u + grad_u * d)/du = phiFace + gradPhiFace * d.
  switch (type)
  {
    case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac *= _test[_i][_qp] * (_vars[component_j]->phiFace()[_j][_qp] +
                               _vars[component_j]->gradPhiFace()[_j][_qp] * d);
      break;
    case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac *= -_test[_i][_qp] * (_vars[component_j]->phiFaceNeighbor()[_j][_qp] +
                                _vars[component_j]->gradPhiFaceNeighbor()[_j][_qp] * d);
      break;
    case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
      jac *= -_test_neighbor[_i][_qp] * (_vars[component_j]->phiFace()[_j][_qp] +
                                         _vars[component_j]->gradPhiFace()[_j][_qp] * d);
      break;
    case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
      jac *= _test_neighbor[_i][_qp] * (_vars[component_j]->phiFaceNeighbor()[_j][_qp] +
                                        _vars[component_j]->gradPhiFaceNeighbor()[_j][_qp] * d);
      break;
  }

  jac *= true_normal_dot_surrogate_normal; // Area correction

  return jac;
}

template <>
Real SCZMInterfaceKernelSmallStrainTempl<false>::calculateDirectionalCorrectionJacobian(
    unsigned int ivar, unsigned int jvar, Moose::DGJacobianType type) const;

template <bool is_ad>
Real
SCZMInterfaceKernelSmallStrainTempl<is_ad>::computeQpJacobian(Moose::DGJacobianType)
{
  return 0;
}

template <>
Real
SCZMInterfaceKernelSmallStrainTempl<false>::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jacobian = computeDResidualDDisplacement(this->_component, type);

  if (_shifted && _directional_correction)
    switch (type)
    {
      case Moose::ElementElement:
        jacobian -= calculateDirectionalCorrectionJacobian(_component, _component, type);
        break;
      case Moose::NeighborNeighbor:
        jacobian += calculateDirectionalCorrectionJacobian(_component, _component, type);
        break;
      case Moose::ElementNeighbor:
      case Moose::NeighborElement:
        break;
    }

  return jacobian;
}

template <bool is_ad>
Real
SCZMInterfaceKernelSmallStrainTempl<is_ad>::computeQpOffDiagJacobian(Moose::DGJacobianType,
                                                                     unsigned int)
{
  return 0;
}

template <>
Real
SCZMInterfaceKernelSmallStrainTempl<false>::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                                     unsigned int jvar)
{
  if (this->getJvarMap()[jvar] < 0)
    return 0;

  Real jacobian = 0;
  for (const auto coupled_component : make_range(this->_ndisp))
    if (jvar == _disp_var[coupled_component])
      jacobian = computeDResidualDDisplacement(coupled_component, type);

  if (_shifted && _directional_correction)
    for (const auto coupled_component : make_range(_ndisp))
      if (jvar == _disp_var[coupled_component])
        switch (type)
        {
          case Moose::ElementElement:
            jacobian -= calculateDirectionalCorrectionJacobian(_component, coupled_component, type);
            break;
          case Moose::NeighborNeighbor:
            jacobian += calculateDirectionalCorrectionJacobian(_component, coupled_component, type);
            break;
          case Moose::ElementNeighbor:
          case Moose::NeighborElement:
            break;
        }

  return jacobian;
}

template <bool is_ad>
Real
SCZMInterfaceKernelSmallStrainTempl<is_ad>::calculateDirectionalCorrectionJacobian(
    unsigned int, unsigned int, Moose::DGJacobianType) const
{
  return 0;
}

template <>
Real
SCZMInterfaceKernelSmallStrainTempl<false>::calculateDirectionalCorrectionJacobian(
    unsigned int ivar, unsigned int jvar, Moose::DGJacobianType type) const
{
  const auto true_normal =
      _shifted ? RealVectorValue(trueNormal()) : RealVectorValue(_normals[_qp]);

  const bool trial_on_element = type == Moose::ElementElement || type == Moose::NeighborElement;
  const auto grad_phi = trial_on_element ? _grad_phi[_j][_qp] : _grad_phi_neighbor[_j][_qp];

  const auto nt_tangent = _normals[_qp] - (true_normal * _normals[_qp]) * true_normal;

  const auto test = (type == Moose::ElementElement || type == Moose::ElementNeighbor)
                        ? _test[_i][_qp]
                        : _test_neighbor[_i][_qp];

  const auto & r4t = (type == Moose::ElementElement || type == Moose::ElementNeighbor)
                         ? (*_Jacobian_mult)[_qp]
                         : (*_Jacobian_mult_neighbor)[_qp];

  const Real origin_part = ElasticityTensorTools::elasticJacobian(
      r4t, ivar, jvar, nt_tangent * test /*transpose later (w^T n^T)*/, grad_phi /*B*/);

  const Real n0 = nt_tangent(0);
  const Real n1 = nt_tangent(1);
  const Real n2 = nt_tangent(2);

  const Real gp0 = grad_phi(0);
  const Real gp1 = grad_phi(1);
  const Real gp2 = grad_phi(2);

  const auto i = ivar;
  const auto k = jvar;

  const Real symmetric_part =
      ((r4t(i, 0, 0, k) * gp0 + r4t(i, 0, 1, k) * gp1 + r4t(i, 0, 2, k) * gp2) * n0 +
       (r4t(i, 1, 0, k) * gp0 + r4t(i, 1, 1, k) * gp1 + r4t(i, 1, 2, k) * gp2) * n1 +
       (r4t(i, 2, 0, k) * gp0 + r4t(i, 2, 1, k) * gp1 + r4t(i, 2, 2, k) * gp2) * n2) *
      test;

  // dP/dF acts on the full displacement gradient. A strain-based tangent instead acts on the
  // symmetric gradient, which contributes the transposed contraction as well.
  Real jacobian = _tangent_is_dpk1_df ? origin_part : (origin_part + symmetric_part) * 0.5;

  if (_volumetric_locking_correction)
  {
    const auto grad_op = gradOpForCoord(_coord_sys,
                                        k,
                                        grad_phi,
                                        trial_on_element ? _phi[_j][_qp] : _phi_neighbor[_j][_qp],
                                        _q_point[_qp]);
    const auto avg_grad_op = StabilizationUtils::elementAverage(
        [this, k, trial_on_element](unsigned int qp)
        {
          return gradOpForCoord(_coord_sys,
                                k,
                                trial_on_element ? _grad_phi[_j][qp] : _grad_phi_neighbor[_j][qp],
                                trial_on_element ? _phi[_j][qp] : _phi_neighbor[_j][qp],
                                _q_point[qp]);
        },
        _JxW,
        _coord);
    const Real volumetric_increment = (avg_grad_op.trace() - grad_op.trace()) / 3.0;

    for (const auto m : make_range(3))
      jacobian += (r4t(i, 0, m, m) * n0 + r4t(i, 1, m, m) * n1 + r4t(i, 2, m, m) * n2) * test *
                  volumetric_increment;
  }

  return jacobian;
}

template class SCZMInterfaceKernelSmallStrainTempl<false>;
template class SCZMInterfaceKernelSmallStrainTempl<true>;
