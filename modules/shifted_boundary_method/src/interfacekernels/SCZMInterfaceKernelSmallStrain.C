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

registerMooseObject("ShiftedBoundaryMethodApp", SCZMInterfaceKernelSmallStrain);

InputParameters
SCZMInterfaceKernelSmallStrain::validParams()
{
  InputParameters params = SCZMInterfaceKernelBase::validParams();

  params.addParam<bool>("consistency", true, "Adding Shifted consistency terms.");
  params.addParam<MaterialPropertyName>(
      "stress", "stress", "Name of the stress tensor material property.");
  params.addParam<MaterialPropertyName>(
      "tangent", "Jacobian_mult", "Name of the material Jacobian tensor property.");
  params.addClassDescription(
      "Shifted CZM Interface kernel to use when using the Small Strain kinematic formulation.");

  return params;
}

SCZMInterfaceKernelSmallStrain::SCZMInterfaceKernelSmallStrain(const InputParameters & parameters)
  : SCZMInterfaceKernelBase(parameters),
    _stress(getMaterialPropertyByName<RankTwoTensor>(getParam<MaterialPropertyName>("stress"))),
    _stress_neighbor(
        getNeighborMaterialPropertyByName<RankTwoTensor>(getParam<MaterialPropertyName>("stress"))),
    _Jacobian_mult(
        getMaterialPropertyByName<RankFourTensor>(getParam<MaterialPropertyName>("tangent"))),
    _Jacobian_mult_neighbor(getNeighborMaterialPropertyByName<RankFourTensor>(
        getParam<MaterialPropertyName>("tangent"))),
    _consistency_term(getParam<bool>("consistency"))
{
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

  // Optional stress-based consistency term restoring traction balance on the true interface.
  if (_consistency_term)
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

  const auto d = surrogateDistance();
  const auto field_correction_jac = [this, component_j, jacsd, d](const bool neighbor)
  {
    return jacsd * ((neighbor ? _vars[component_j]->gradPhiFaceNeighbor()[_j][_qp]
                              : _vars[component_j]->gradPhiFace()[_j][_qp]) *
                    d);
  };

  if (!_shifted)
  {
    switch (type)
    {
      case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
        jac *= _test[_i][_qp] * _vars[component_j]->phiFace()[_j][_qp];
        if (_field_correction)
          jac += _test[_i][_qp] * field_correction_jac(/*neighbor=*/false);
        break;
      case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
        jac *= -_test[_i][_qp] * _vars[component_j]->phiFaceNeighbor()[_j][_qp];
        if (_field_correction)
          jac -= _test[_i][_qp] * field_correction_jac(/*neighbor=*/true);
        break;
      case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
        jac *= -_test_neighbor[_i][_qp] * _vars[component_j]->phiFace()[_j][_qp];
        if (_field_correction)
          jac -= _test_neighbor[_i][_qp] * field_correction_jac(/*neighbor=*/false);
        break;
      case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
        jac *= _test_neighbor[_i][_qp] * _vars[component_j]->phiFaceNeighbor()[_j][_qp];
        if (_field_correction)
          jac += _test_neighbor[_i][_qp] * field_correction_jac(/*neighbor=*/true);
        break;
    }
    return jac;
  }

  const auto true_normal = RealVectorValue(trueNormal());
  const auto true_normal_dot_surrogate_normal = true_normal * _normals[_qp];

  // Shifted derivative uses d(u + grad_u * d)/du = phiFace + gradPhiFace * d.
  switch (type)
  {
    case Moose::ElementElement: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac *= _test[_i][_qp] * (_vars[component_j]->phiFace()[_j][_qp] +
                               _vars[component_j]->gradPhiFace()[_j][_qp] * d);
      if (_field_correction)
        jac += _test[_i][_qp] * field_correction_jac(/*neighbor=*/false);
      break;
    case Moose::ElementNeighbor: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac *= -_test[_i][_qp] * (_vars[component_j]->phiFaceNeighbor()[_j][_qp] +
                                _vars[component_j]->gradPhiFaceNeighbor()[_j][_qp] * d);
      if (_field_correction)
        jac -= _test[_i][_qp] * field_correction_jac(/*neighbor=*/true);
      break;
    case Moose::NeighborElement: // Residual_sign 1  ddeltaU_ddisp sign -1;
      jac *= -_test_neighbor[_i][_qp] * (_vars[component_j]->phiFace()[_j][_qp] +
                                         _vars[component_j]->gradPhiFace()[_j][_qp] * d);
      if (_field_correction)
        jac -= _test_neighbor[_i][_qp] * field_correction_jac(/*neighbor=*/false);
      break;
    case Moose::NeighborNeighbor: // Residual_sign 1  ddeltaU_ddisp sign 1;
      jac *= _test_neighbor[_i][_qp] * (_vars[component_j]->phiFaceNeighbor()[_j][_qp] +
                                        _vars[component_j]->gradPhiFaceNeighbor()[_j][_qp] * d);
      if (_field_correction)
        jac += _test_neighbor[_i][_qp] * field_correction_jac(/*neighbor=*/true);
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

  if (_shifted && _consistency_term)
    switch (type)
    {
      case Moose::ElementElement:
        jacobian -= calculateConsistencyJacobian(_component, _component, type);
        break;
      case Moose::NeighborNeighbor:
        jacobian += calculateConsistencyJacobian(_component, _component, type);
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

  if (_shifted && _consistency_term)
    for (unsigned int coupled_component = 0; coupled_component < _ndisp; ++coupled_component)
      if (jvar == _disp_var[coupled_component])
        switch (type)
        {
          case Moose::ElementElement:
            jacobian -= calculateConsistencyJacobian(_component, coupled_component, type);
            break;
          case Moose::NeighborNeighbor:
            jacobian += calculateConsistencyJacobian(_component, coupled_component, type);
            break;
          case Moose::ElementNeighbor:
          case Moose::NeighborElement:
            break;
        }

  return jacobian;
}

Real
SCZMInterfaceKernelSmallStrain::calculateConsistencyJacobian(unsigned int ivar,
                                                             unsigned int jvar,
                                                             Moose::DGJacobianType type) const
{
  const auto true_normal =
      _shifted ? RealVectorValue(trueNormal()) : RealVectorValue(_normals[_qp]);

  const auto grad_phi = (type == Moose::ElementElement || type == Moose::NeighborElement)
                            ? _grad_phi[_j][_qp]
                            : _grad_phi_neighbor[_j][_qp];

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

  return (origin_part + symmetric_part) * 0.5;
}
