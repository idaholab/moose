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

InputParameters
SCZMInterfaceKernelSmallStrain::validParams()
{
  InputParameters params = SCZMInterfaceKernelBase::validParams();

  params.addParam<bool>("directional_correction", true, "Add the directional correction terms.");
  params.addParam<MaterialPropertyName>(
      "stress", "stress", "Name of the stress tensor material property.");
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
      "Whether to apply volumetric locking correction to the directional correction term.");
  params.addClassDescription(
      "Shifted CZM Interface kernel to use when using the Small Strain kinematic formulation.");

  return params;
}

SCZMInterfaceKernelSmallStrain::SCZMInterfaceKernelSmallStrain(const InputParameters & parameters)
  : SCZMInterfaceKernelBase(parameters),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name +
                                                     getParam<MaterialPropertyName>("stress"))),
    _stress_neighbor(getNeighborMaterialPropertyByName<RankTwoTensor>(
        _base_name + getParam<MaterialPropertyName>("stress"))),
    _Jacobian_mult(getMaterialPropertyByName<RankFourTensor>(
        _base_name + getParam<MaterialPropertyName>("tangent"))),
    _Jacobian_mult_neighbor(getNeighborMaterialPropertyByName<RankFourTensor>(
        _base_name + getParam<MaterialPropertyName>("tangent"))),
    _tangent_is_dpk1_df(false),
    _directional_correction(getParam<bool>("directional_correction")),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction"))
{
  const auto & tangent_name = getParam<MaterialPropertyName>("tangent");
  const auto & tangent_definition = getParam<MooseEnum>("tangent_definition");
  if (tangent_definition == "auto")
  {
    if (tangent_name == "Jacobian_mult")
      _tangent_is_dpk1_df = false;
    else if (tangent_name == "pk1_jacobian")
      _tangent_is_dpk1_df = true;
    else
      paramError("tangent_definition",
                 "Cannot infer the mathematical definition of custom tangent property '",
                 tangent_name,
                 "'. Set 'tangent_definition' to 'stress_wrt_strain' or "
                 "'pk1_wrt_deformation_gradient'.");
  }
  else
    _tangent_is_dpk1_df = tangent_definition == "pk1_wrt_deformation_gradient";
}

Real
SCZMInterfaceKernelSmallStrain::computeQpResidual(Moose::DGResidualType type)
{
  // Non-shifted: same as base (pure traction DG term).
  if (!_shifted)
    return SCZMInterfaceKernelBase::computeQpResidual(type);

  const RealVectorValue true_normal(trueNormal());
  const Real true_normal_dot_surrogate_normal = true_normal * _normals[_qp];

  // Start from the base traction residual and apply the shifted (area) scaling.
  Real residual = SCZMInterfaceKernelBase::computeQpResidual(type);
  residual *= true_normal_dot_surrogate_normal;

  // Optional stress-based directional correction restoring traction balance on the true interface.
  if (_directional_correction)
  {
    const RealVectorValue stress = _stress[_qp].row(_component);
    const RealVectorValue stress_neigh = _stress_neighbor[_qp].row(_component);

    const RealVectorValue nt_tangent =
        _normals[_qp] - true_normal_dot_surrogate_normal * true_normal;

    switch (type)
    {
      case Moose::Element:
        residual -= _test[_i][_qp] * (stress * nt_tangent);
        break;

      case Moose::Neighbor:
        residual += _test_neighbor[_i][_qp] * (stress_neigh * nt_tangent);
        break;

      default:
        break;
    }
  }

  return residual;
}

Real
SCZMInterfaceKernelSmallStrain::computeDResidualDDisplacement(
    const unsigned int & component_j, const Moose::DGJacobianType & type) const
{
  const auto jacsd = _dtraction_djump_global[_qp](_component, component_j);
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

Real
SCZMInterfaceKernelSmallStrain::computeQpJacobian(Moose::DGJacobianType type)
{
  // The area correction is taken care of by computeDResidualDDisplacement.
  Real jacobian = SCZMInterfaceKernelBase::computeQpJacobian(type);

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

Real
SCZMInterfaceKernelSmallStrain::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                         unsigned int jvar)
{
  Real jacobian = SCZMInterfaceKernelBase::computeQpOffDiagJacobian(type, jvar);

  if (_shifted && _directional_correction)
    for (unsigned int coupled_component = 0; coupled_component < _ndisp; ++coupled_component)
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

Real
SCZMInterfaceKernelSmallStrain::calculateDirectionalCorrectionJacobian(
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
                         ? _Jacobian_mult[_qp]
                         : _Jacobian_mult_neighbor[_qp];

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
