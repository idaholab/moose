//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarScalarBase.h"

// MOOSE includes
#include "Assembly.h"
#include "SystemBase.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"

InputParameters
MortarScalarBase::validParams()
{
  InputParameters params = MortarConstraint::validParams();
  // This parameter can get renamed in derived class to a more relevant variable name
  params.addCoupledVar("scalar_variable", "Primary coupled scalar variable");
  params.addParam<bool>("compute_scalar_residuals", true, "Whether to compute scalar residuals");
  return params;
}

MortarScalarBase::MortarScalarBase(const InputParameters & parameters)
  : MortarConstraint(parameters),
    _use_scalar(isParamValid("scalar_variable") ? true : false),
    _compute_scalar_residuals(!_use_scalar ? false : getParam<bool>("compute_scalar_residuals")),
    _kappa_var_ptr(_use_scalar ? getScalarVar("scalar_variable", 0) : nullptr),
    _kappa_var(_use_scalar ? _kappa_var_ptr->number() : 0),
    _k_order(_use_scalar ? _kappa_var_ptr->order() : 0),
    _kappa(_use_scalar ? (_is_implicit ? _kappa_var_ptr->sln() : _kappa_var_ptr->slnOld()) : _zero)
{
}

void
MortarScalarBase::computeResidual()
{
  MortarConstraintBase::computeResidual();

  if (!_compute_scalar_residuals)
    return;

  std::vector<Real> scalar_residuals(_k_order);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    initScalarQpResidual();
    for (_h = 0; _h < _k_order; _h++)
      scalar_residuals[_h] += _JxW_msm[_qp] * _coord[_qp] * computeScalarQpResidual();
  }

  addResiduals(
      _assembly, scalar_residuals, _kappa_var_ptr->dofIndices(), _kappa_var_ptr->scalingFactor());
}

void
MortarScalarBase::computeJacobian()
{
  // d-_var-residual / d-_var and d-_var-residual / d-jvar
  MortarConstraintBase::computeJacobian();

  if (!_use_scalar)
    return;

  // Get the list of coupled scalar vars and compute their off-diag jacobians
  const auto & coupled_scalar_vars = getCoupledMooseScalarVars();

  // Handle ALL d-_var-residual / d-scalar columns like computeOffDiagJacobianScalar
  if (_compute_primal_residuals)
    // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
    for (const auto & svariable : coupled_scalar_vars)
      if (_sys.hasScalarVariable(svariable->name()))
      {
        // Compute the jacobian for the secondary interior primal dofs
        computeOffDiagJacobianScalar(Moose::MortarType::Secondary, svariable->number());
        // Compute the jacobian for the primary interior primal dofs.
        computeOffDiagJacobianScalar(Moose::MortarType::Primary, svariable->number());
      }

  if (_compute_lm_residuals)
    // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
    for (const auto & svariable : coupled_scalar_vars)
      if (_sys.hasScalarVariable(svariable->name()))
        // Compute the jacobian for the lower dimensional LM dofs (if we even have an LM variable)
        computeOffDiagJacobianScalar(Moose::MortarType::Lower, svariable->number());

  if (_compute_scalar_residuals)
  {
    // Handle ALL d-_kappa-residual / d-_var and d-_kappa-residual / d-jvar columns
    computeScalarOffDiagJacobian();

    // Do: d-_kappa-residual / d-_kappa and d-_kappa-residual / d-jvar,
    // only want to process only nl-variables (not aux ones)
    for (const auto & svariable : coupled_scalar_vars)
    {
      if (_sys.hasScalarVariable(svariable->name()))
      {
        const unsigned int svar_num = svariable->number();
        if (svar_num == _kappa_var)
          computeScalarJacobian(); // d-_kappa-residual / d-_kappa
        else
          computeScalarOffDiagJacobianScalar(svar_num); // d-_kappa-residual / d-svar
      }
    }
  }
}

void
MortarScalarBase::computeScalarJacobian()
{
  _local_ke.resize(_k_order, _k_order);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    initScalarQpJacobian(_kappa_var);
    for (_h = 0; _h < _k_order; _h++)
      for (_l = 0; _l < _k_order; _l++)
        _local_ke(_h, _l) += _JxW_msm[_qp] * _coord[_qp] * computeScalarQpJacobian();
  }

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->scalingFactor());
}

void
MortarScalarBase::computeScalarOffDiagJacobian()
{
  typedef Moose::MortarType MType;
  std::array<MType, 3> mortar_types = {{MType::Secondary, MType::Primary, MType::Lower}};

  auto & ce = _assembly.scalarFieldCouplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableScalar & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    const unsigned int ivar_num = ivariable.number();
    const unsigned int jvar_num = jvariable.number();

    if (ivar_num != _kappa_var) // only do the row for _kappa_var in this object
      continue;

    // Load shape functions of different types for easy access; identical to MortarConstraint.C
    std::array<size_t, 3> shape_space_sizes{{jvariable.dofIndices().size(),
                                             jvariable.dofIndicesNeighbor().size(),
                                             jvariable.dofIndicesLower().size()}};
    std::array<const VariablePhiValue *, 3> phis;
    std::array<const VariablePhiGradient *, 3> grad_phis;
    std::array<const VectorVariablePhiValue *, 3> vector_phis;
    std::array<const VectorVariablePhiGradient *, 3> vector_grad_phis;
    if (jvariable.isVector())
    {
      const auto & temp_var = static_cast<MooseVariableFE<RealVectorValue> &>(jvariable);
      vector_phis = {{&temp_var.phiFace(), &temp_var.phiFaceNeighbor(), &temp_var.phiLower()}};
      vector_grad_phis = {
          {&temp_var.gradPhiFace(), &temp_var.gradPhiFaceNeighbor(), &temp_var.gradPhiLower()}};
    }
    else
    {
      const auto & temp_var = static_cast<MooseVariableFE<Real> &>(jvariable);
      phis = {{&temp_var.phiFace(), &temp_var.phiFaceNeighbor(), &temp_var.phiLower()}};
      grad_phis = {
          {&temp_var.gradPhiFace(), &temp_var.gradPhiFaceNeighbor(), &temp_var.gradPhiLower()}};
    }

    // Loop over 3 types of spatial variables, find out what jvar_num is
    for (MooseIndex(3) type_index = 0; type_index < 3; ++type_index)
    {
      const auto mortar_type = mortar_types[type_index];
      const auto shape_space_size = shape_space_sizes[type_index];
      std::vector<dof_id_type> dof_indices;
      switch (mortar_type)
      {
        case MType::Secondary:
          dof_indices = jvariable.dofIndices();
          break;

        case MType::Primary:
          dof_indices = jvariable.dofIndicesNeighbor();
          break;

        case MType::Lower:
          dof_indices = jvariable.dofIndicesLower();
          break;
      }

      /// Set the proper phis
      if (jvariable.isVector())
      {
        _vector_phi = vector_phis[type_index];
        _vector_grad_phi = vector_grad_phis[type_index];
      }
      else
      {
        _phi = phis[type_index];
        _grad_phi = grad_phis[type_index];
      }

      _local_ke.resize(_k_order, shape_space_size);

      for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
      {
        initScalarQpOffDiagJacobian(mortar_type, jvar_num);
        const Real dV = _JxW_msm[_qp] * _coord[_qp];
        for (_h = 0; _h < _k_order; _h++)
        {
          for (_j = 0; _j < shape_space_size; _j++)
          {
            _local_ke(_h, _j) += computeScalarQpOffDiagJacobian(mortar_type, jvar_num) * dV;
          }
        }
      }

      addJacobian(_assembly,
                  _local_ke,
                  _kappa_var_ptr->dofIndices(),
                  dof_indices,
                  _kappa_var_ptr->scalingFactor());
    }
  }
}

void
MortarScalarBase::computeOffDiagJacobianScalar(Moose::MortarType mortar_type, unsigned int svar_num)
{
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

  // Get dofs and order of this scalar; at least one will be _kappa_var
  const auto & svar = _sys.getScalarVariable(_tid, svar_num);
  const unsigned int s_order = svar.order();

  _local_ke.resize(test_space_size, s_order);

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    initScalarQpOffDiagJacobian(mortar_type, svar_num);
    const Real dV = _JxW_msm[_qp] * _coord[_qp];
    for (_h = 0; _h < s_order; _h++)
      for (_i = 0; _i < test_space_size; _i++)
      { // This assumes Galerkin, i.e. the test and trial functions are the
        // same
        _j = _i;
        _local_ke(_i, _h) += computeQpOffDiagJacobianScalar(mortar_type, svar_num) * dV;
      }
  }

  addJacobian(_assembly, _local_ke, dof_indices, svar.dofIndices(), scaling_factor);
}

void
MortarScalarBase::computeScalarOffDiagJacobianScalar(const unsigned int svar_num)
{
  // Get dofs and order of this scalar; will NOT be _kappa_var
  const auto & svar = _sys.getScalarVariable(_tid, svar_num);
  const unsigned int s_order = svar.order();

  _local_ke.resize(_k_order, s_order);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    initScalarQpJacobian(svar_num);
    for (_h = 0; _h < _k_order; _h++)
      for (_l = 0; _l < s_order; _l++)
        _local_ke(_h, _l) +=
            _JxW_msm[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobianScalar(svar_num);
  }

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              svar.dofIndices(),
              _kappa_var_ptr->scalingFactor());
}
