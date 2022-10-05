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
  params.addParam<VariableName>("scalar_variable", "Primary coupled scalar variable");
  // This name is fixed and required to be equal to the previous parameter; need to add error checks...
  params.addCoupledVar("coupled_scalar", "Repeat name of scalar variable to ensure dependency");
  return params;
}

MortarScalarBase::MortarScalarBase(const InputParameters & parameters)
  : MortarConstraint(parameters),
    _use_scalar(isParamValid("scalar_variable")
             ? true : false),
    _kappa_dummy(),
    _kappa_var_ptr(_use_scalar
        ? &_sys.getScalarVariable(_tid, parameters.get<VariableName>("scalar_variable"))
        : nullptr),
    _kappa_var(_use_scalar ? _kappa_var_ptr->number() : 0),
    _k_order(_use_scalar ? _kappa_var_ptr->order() : 0),
    _kappa(_use_scalar
        ? (_is_implicit ? _kappa_var_ptr->sln() : _kappa_var_ptr->slnOld())
        : _kappa_dummy)
{
  // add some error checks here
}

void
MortarScalarBase::computeResidual()
{
  MortarConstraintBase::computeResidual();

  if (_use_scalar)
  {
    std::vector<Real> scalar_residuals(_k_order);
    for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    {
      initScalarQpResidual();
      for (_h = 0; _h < _k_order; _h++)
      {
        scalar_residuals[_h] +=
            _JxW_msm[_qp] * _coord[_qp] * computeScalarQpResidual();
      }
    }
    _assembly.processResiduals(scalar_residuals,
                              _kappa_var_ptr->dofIndices(),
                              _vector_tags,
                              _kappa_var_ptr->scalingFactor());
  }
}

void
MortarScalarBase::computeJacobian()
{
  // d-_var-residual / d-_var and d-_var-residual / d-jvar
  MortarConstraintBase::computeJacobian();

  if (_use_scalar)
  {
    // Get the list of coupled scalar vars and compute their off-diag jacobians
    const auto & coupled_scalar_vars = getCoupledMooseScalarVars();  
    
    // Handle ALL d-_var-residual / d-scalar columns like computeOffDiagJacobianScalar
    if (_compute_primal_residuals)
    {
        // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
        for (const auto & jvariable : coupled_scalar_vars)
        {
          if (_sys.hasScalarVariable(jvariable->name()))
          {
            // Compute the jacobian for the secondary interior primal dofs
            computeOffDiagJacobianScalar(Moose::MortarType::Secondary, jvariable->number());
            // Compute the jacobian for the primary interior primal dofs.
            computeOffDiagJacobianScalar(Moose::MortarType::Primary, jvariable->number());
          }
        }
    }
    if (_compute_lm_residuals)
        // Do: dvar / dscalar_var, only want to process only nl-variables (not aux ones)
        for (const auto & jvariable : coupled_scalar_vars)
          if (_sys.hasScalarVariable(jvariable->name()))
            // Compute the jacobian for the lower dimensional LM dofs (if we even have an LM variable)
            computeOffDiagJacobianScalar(Moose::MortarType::Lower, jvariable->number());

    // Handle ALL d-_kappa-residual / d-_var and d-_kappa-residual / d-jvar columns
    auto & ce = _assembly.scalarFieldCouplingEntries();
    for (const auto & it : ce)
    {
      MooseVariableScalar & ivariable = *(it.first);
      MooseVariableFEBase & jvariable = *(it.second);

      unsigned int ivar = ivariable.number();
      unsigned int jvar_num = jvariable.number();

      if (ivar != _kappa_var) // only do the row for _kappa_var in this object
        continue;

      if (_compute_primal_residuals)
      {
        // Compute the jacobian for the secondary interior primal dofs
        computeScalarOffDiagJacobian(Moose::MortarType::Secondary, jvar_num);
        // Compute the jacobian for the primary interior primal dofs.
        computeScalarOffDiagJacobian(Moose::MortarType::Primary, jvar_num);
      }
      if (_compute_lm_residuals)
        // Compute the jacobian for the lower dimensional LM dofs (if we even have an LM variable)
        computeScalarOffDiagJacobian(Moose::MortarType::Lower, jvar_num);
    }
    
    // Do: d-_kappa-residual / d-_kappa and d-_kappa-residual / d-jvar, 
    // only want to process only nl-variables (not aux ones)
    for (const auto & jvariable : coupled_scalar_vars)
    {
      if (_sys.hasScalarVariable(jvariable->name()))
      {
        const unsigned int jvar_num = jvariable->number();
        if (jvar_num == _kappa_var)
          computeScalarJacobian(); // d-_kappa-residual / d-_kappa
        else
          computeScalarOffDiagJacobianScalar(jvar_num); // d-_kappa-residual / d-jvar
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

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 _kappa_var_ptr->dofIndices(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);
}

void
MortarScalarBase::computeScalarOffDiagJacobian(Moose::MortarType mortar_type, unsigned int jvar_num)
{

  // Assumes all coupling variables have same test functions as var/primary/secondary

  unsigned int test_space_size = 0;
  std::vector<dof_id_type> dof_indices;
  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
      test_space_size = _test_secondary.size();
      dof_indices = _secondary_var.dofIndices();
      break;

    case Moose::MortarType::Primary:
      test_space_size = _test_primary.size();
      dof_indices = _primary_var.dofIndicesNeighbor();
      break;

    case Moose::MortarType::Lower:
      mooseAssert(_var, "LM variable is null");
      test_space_size = _test.size();
      dof_indices = _var->dofIndicesLower();
      break;
  }

  _local_ke.resize(_k_order, test_space_size);

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    initScalarQpOffDiagJacobian(mortar_type, jvar_num);
    const Real dV = _JxW_msm[_qp] * _coord[_qp];
    for (_h = 0; _h < _k_order; _h++)
    {
      for (_i = 0; _i < test_space_size; _i++)
      { // This assumes Galerkin, i.e. the test and trial functions are the
        // same
        _j = _i;
        _local_ke(_h, _i) += computeScalarQpOffDiagJacobian(mortar_type, jvar_num) * dV;
      }
    }
  }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(
        _local_ke, _kappa_var_ptr->dofIndices(), dof_indices, _kappa_var_ptr->scalingFactor(), matrix_tag);
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
    {
      for (_i = 0; _i < test_space_size; _i++)
      { // This assumes Galerkin, i.e. the test and trial functions are the
        // same
        _j = _i;
        _local_ke(_i, _h) += computeQpOffDiagJacobianScalar(mortar_type, svar_num) * dV;
      }
    }
  }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 dof_indices,
                                 svar.dofIndices(),
                                 scaling_factor,
                                 matrix_tag);
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
        _local_ke(_h, _l) += _JxW_msm[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobianScalar(svar_num);
  }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 svar.dofIndices(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);
}
