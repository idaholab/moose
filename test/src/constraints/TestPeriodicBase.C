//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestPeriodicBase.h"

registerMooseObject("MooseTestApp", TestPeriodicBase);

InputParameters
TestPeriodicBase::validParams()
{
  InputParameters params = MortarConstraint::validParams();
  params.addRequiredCoupledVar("kappa", "Unknown scalar averaging variable");
  params.addRequiredCoupledVar("kappa_aux", "Controlled scalar averaging variable");
  params.addParam<Real>("pen_scale", 1.0, "Increase or decrease the penalty");

  return params;
}

TestPeriodicBase::TestPeriodicBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<MortarConstraint>(parameters),
    _temp_jump_global(),
    _tau_s(),
    _kappa_var(coupledScalar("kappa")),
    _k_order(getScalarVar("kappa", 0)->order()),
    _kappa_var_ptr(getScalarVar("kappa", 0)),
    _kappa(coupledScalarValue("kappa")),
    _kappa_aux_var(coupledScalar("kappa_aux")),
    _ka_order(getScalarVar("kappa_aux", 0)->order()),
    _kappa_aux(coupledScalarValue("kappa_aux")),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side_volume(_assembly.sideElemVolume()),
    _pen_scale(getParam<Real>("pen_scale")),
    _ad_u_secondary(_secondary_var.adSln()),
    _ad_u_primary(_primary_var.adSlnNeighbor())
{
}

// Compute the stability parameters to use for all quadrature points
void
TestPeriodicBase::precalculateResidual()
{
  precalculateStability();
}
void
TestPeriodicBase::precalculateJacobian()
{
  precalculateStability();
}

void
TestPeriodicBase::computeResidualScalar()
{
  std::vector<Real> scalar_residuals(_k_order);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    precalculateMaterial();
    for (_h = 0; _h < _k_order; _h++)
    {
      scalar_residuals[_h] +=
          _JxW_msm[_qp] * _coord[_qp] * MetaPhysicL::raw_value(computeQpResidualScalar());
      scalar_residuals[_h] +=
          _JxW_msm[_qp] * _coord[_qp] * MetaPhysicL::raw_value(computeQpResidualScalarScalar());
    }
  }
  _assembly.processResiduals(scalar_residuals,
                             _kappa_var_ptr->dofIndices(),
                             _vector_tags,
                             _kappa_var_ptr->scalingFactor());
}

void
TestPeriodicBase::computeJacobianScalar()
{
  std::vector<ADReal> scalar_residuals(_k_order);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    precalculateMaterial();
    for (_h = 0; _h < _k_order; _h++)
    {
      scalar_residuals[_h] += _JxW_msm[_qp] * _coord[_qp] * computeQpResidualScalar();
      scalar_residuals[_h] += _JxW_msm[_qp] * _coord[_qp] * computeQpResidualScalarScalar();
    }
  }

  // Handle secondary
  _local_ke.resize(_k_order, _secondary_var.dofIndices().size());
  for (const auto h : make_range(_k_order))
    for (const auto j : index_range(_secondary_var.dofIndices()))
    {
      const auto global_dof_index = _secondary_var.dofIndices()[j];
      _local_ke(h, j) = scalar_residuals[h].derivatives()[global_dof_index];
    }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 _secondary_var.dofIndices(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);

  // Handle primary
  _local_ke.resize(_k_order, _primary_var.dofIndicesNeighbor().size());
  for (const auto h : make_range(_k_order))
    for (const auto j : index_range(_primary_var.dofIndicesNeighbor()))
    {
      const auto global_dof_index = _primary_var.dofIndicesNeighbor()[j];
      _local_ke(h, j) = scalar_residuals[h].derivatives()[global_dof_index];
    }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 _primary_var.dofIndicesNeighbor(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);

  // Handle scalar
  mooseAssert(_k_order == _kappa_var_ptr->dofIndices().size(),
              "This should be obvious at this point");
  _local_ke.resize(_k_order, _k_order);
  for (const auto h : make_range(_k_order))
    for (const auto j : make_range(_k_order))
    {
      const auto global_dof_index = _kappa_var_ptr->dofIndices()[j];
      _local_ke(h, j) = scalar_residuals[h].derivatives()[global_dof_index];
    }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 _kappa_var_ptr->dofIndices(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);
}

void
TestPeriodicBase::computeOffDiagJacobianScalar(Moose::MortarType mortar_type, unsigned int jvar)
{
  if (jvar != _kappa_var)
    return;

  unsigned int test_space_size = 0;
  std::vector<dof_id_type> dof_indices;
  Real scaling_factor = 1;
  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
      test_space_size = _test_secondary.size();
      dof_indices = _secondary_var.dofIndices();
      scaling_factor = _secondary_var.scalingFactor();
      break;

    case Moose::MortarType::Primary:
      test_space_size = _test_primary.size();
      dof_indices = _primary_var.dofIndicesNeighbor();
      scaling_factor = _primary_var.scalingFactor();
      break;

    case Moose::MortarType::Lower:
      mooseAssert(_var, "LM variable is null");
      test_space_size = _test.size();
      dof_indices = _var->dofIndicesLower();
      scaling_factor = _var->scalingFactor();
      break;
  }

  // DenseMatrix<Number> & ken = _assembly.jacobianBlock(test_space_var, jvar);
  // DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, test_space_var);

  // for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  // {
  //   const Real dV = _JxW_msm[_qp] * _coord[_qp];
  //   for (_h = 0; _h < _k_order; _h++)
  //   {
  //     for (_i = 0; _i < test_space_size; _i++)
  //     { // This assumes Galerkin, i.e. the test and trial functions are the
  //       // same
  //       _j = _i;
  //       ken(_i, _h) += computeQpBaseJacobian(mortar_type, jvar) * dV;
  //       kne(_h, _i) += computeQpConstraintJacobian(mortar_type, jvar) * dV;
  //     }
  //   }
  // }

  _local_ke.resize(test_space_size, _k_order);

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    precalculateMaterial();
    const Real dV = _JxW_msm[_qp] * _coord[_qp];
    for (_h = 0; _h < _k_order; _h++)
    {
      for (_i = 0; _i < test_space_size; _i++)
      { // This assumes Galerkin, i.e. the test and trial functions are the
        // same
        _j = _i;
        _local_ke(_i, _h) += computeQpBaseJacobian(mortar_type, jvar) * dV;
      }
    }
  }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(
        _local_ke, dof_indices, _kappa_var_ptr->dofIndices(), scaling_factor, matrix_tag);

  // _local_ke.resize(_k_order, test_space_size);

  // for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  // {
  //   const Real dV = _JxW_msm[_qp] * _coord[_qp];
  //   for (_h = 0; _h < _k_order; _h++)
  //   {
  //     for (_i = 0; _i < test_space_size; _i++)
  //     { // This assumes Galerkin, i.e. the test and trial functions are the
  //       // same
  //       _j = _i;
  //       _local_ke(_h, _i) += computeQpConstraintJacobian(mortar_type, jvar) * dV;
  //     }
  //   }
  // }

  // for (const auto & matrix_tag : _matrix_tags)
  //   _assembly.cacheJacobianBlock(_local_ke,
  //                                _kappa_var_ptr->dofIndices(),
  //                                dof_indices,
  //                                _kappa_var_ptr->scalingFactor(),
  //                                matrix_tag);
}

Real
TestPeriodicBase::computeQpResidual(const Moose::MortarType mortar_type)
{

  // precalculateMaterial();

  /// Compute penalty parameter times x-jump times average heat flux

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  RealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  Real r = (_pen_scale * _tau_s) * (kappa_vec * dx);
  // Real r = (_pen_scale * pencoef / h_elem) * ((_phys_points_secondary[_qp](0) -
  // _phys_points_primary[_qp](0))*_kappa[0]
  //               + (_phys_points_secondary[_qp](1) - _phys_points_primary[_qp](1))*_kappa[1]
  //               + (_phys_points_secondary[_qp](2) - _phys_points_primary[_qp](2))*_kappa[2]);

  switch (mortar_type)
  {
    // comment
    case Moose::MortarType::Secondary:
      r *= _test_secondary[_i][_qp];
      break;
    case Moose::MortarType::Primary:
      r *= -_test_primary[_i][_qp];
      break;
    case Moose::MortarType::Lower:
      return 0;
    default:
      return 0;
  }
  return r;
}

ADReal
TestPeriodicBase::computeQpResidualScalar()
{

  /// Stability/penalty term for residual of scalar variable
  ADReal r = (_pen_scale * _tau_s) * _temp_jump_global;
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  RealVectorValue hack(7.0, 3.0, 0.0);

  r *= -dx(_h);

  return r / hack(_h);
}

ADReal
TestPeriodicBase::computeQpResidualScalarScalar()
{

  /// Stability/penalty term for residual of scalar variable
  ADRealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  Moose::derivInsert(kappa_vec(0).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
  Moose::derivInsert(kappa_vec(1).derivatives(), _kappa_var_ptr->dofIndices()[1], 1);
  RealVectorValue kappa_aux_vec(_kappa_aux[0], _kappa_aux[1], 0);
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  RealVectorValue hack(7.0, 3.0, 0.0);

  ADReal r = 0.0;
  r += dx(_h) * (_pen_scale * _tau_s) * (kappa_vec * dx);
  r -= dx(_h) * (kappa_aux_vec * _normals[_qp]);

  return r / hack(_h);
}

// Real
// TestPeriodicBase::computeQpJacobianScalarScalar()
// {

//   /// Stability/penalty term for Jacobian of scalar variable
//   RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
//   RealVectorValue hack(7.0, 3.0, 0.0);

//   Real jac = dx(_h) * (_pen_scale * _tau_s) * dx(_l);

//   return jac / hack(_h);
// }

Real
TestPeriodicBase::computeQpBaseJacobian(const Moose::MortarType mortar_type,
                                        const unsigned int /*jvar*/)
{

  // precalculateMaterial();

  /// Stability/penalty term for Jacobian

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real jac = (_pen_scale * _tau_s);

  switch (mortar_type)
  {

    case Moose::MortarType::Secondary: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac *= _test_secondary[_i][_qp] * dx(_h);
      break;
    case Moose::MortarType::Primary: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac *= -_test_primary[_i][_qp] * dx(_h);
      break;

    default:
      return 0;
  }
  return jac;
}

Real
TestPeriodicBase::computeQpConstraintJacobian(const Moose::MortarType mortar_type,
                                              const unsigned int /*jvar*/)
{

  // precalculateMaterial(); // may not be needed...

  /// Stability/penalty term for Jacobian
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real jac = (_pen_scale * _tau_s);

  switch (mortar_type)
  {
    case Moose::MortarType::Secondary: // Residual_sign -1  ddeltaU_ddisp sign 1;
                                       // This assumes Galerkin, i.e. (*_phi)[_i][_qp] =
                                       // _test_secondary[_i][_qp]
      jac *= _test_secondary[_i][_qp] * dx(_h);
      break;
    case Moose::MortarType::Primary: // Residual_sign -1  ddeltaU_ddisp sign -1;
                                     // This assumes Galerkin, i.e. (*_phi)[_i][_qp] =
                                     // _test_primary[_i][_qp]
      jac *= -_test_primary[_i][_qp] * dx(_h);
      break;

    default:
      return 0;
  }
  return jac;
}

void
TestPeriodicBase::precalculateStability()
{
  // const unsigned int elem_b_order = _secondary_var.order();
  // double h_elem =
  //     _current_elem_volume / _current_side_volume * 1. / Utility::pow<2>(elem_b_order);
  // h_elem = 10.0;
  const double h_elem = 1.0;

  _tau_s = (pencoef / h_elem);
}

// Compute temperature jump and flux average/jump
void
TestPeriodicBase::precalculateMaterial()
{
  _temp_jump_global = (_ad_u_primary[_qp] - _ad_u_secondary[_qp]);
}
