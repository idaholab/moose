//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMortarConstraint.h"

// MOOSE includes
#include "MooseVariable.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "ADUtils.h"

InputParameters
ADMortarConstraint::validParams()
{
  InputParameters params = MortarConstraintBase::validParams();
  return params;
}

ADMortarConstraint::ADMortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _lambda_dummy(),
    _lambda(_var ? _var->adSlnLower() : _lambda_dummy),
    _u_secondary(_secondary_var.adSln()),
    _u_primary(_primary_var.adSlnNeighbor()),
    _grad_u_secondary(_secondary_var.adGradSln()),
    _grad_u_primary(_primary_var.adGradSlnNeighbor())
{
  _subproblem.haveADObjects(true);
}

void
ADMortarConstraint::computeResidual(Moose::MortarType mortar_type)
{
  unsigned int test_space_size = 0;
  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
      prepareVectorTag(_assembly, _secondary_var.number());
      test_space_size = _test_secondary.size();
      break;

    case Moose::MortarType::Primary:
      prepareVectorTagNeighbor(_assembly, _primary_var.number());
      test_space_size = _test_primary.size();
      break;

    case Moose::MortarType::Lower:
      mooseAssert(_var, "LM variable is null");
      prepareVectorTagLower(_assembly, _var->number());
      test_space_size = _test.size();
      break;
  }

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (_i = 0; _i < test_space_size; _i++)
      _local_re(_i) += raw_value(_JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type));

  accumulateTaggedLocalResidual();
}

void
ADMortarConstraint::computeJacobian(Moose::MortarType mortar_type)
{
  std::vector<DualReal> residuals;
  size_t test_space_size = 0;
  typedef Moose::ConstraintJacobianType JType;
  typedef Moose::MortarType MType;
  std::vector<JType> jacobian_types;
  std::vector<dof_id_type> dof_indices;

  switch (mortar_type)
  {
    case MType::Secondary:
      dof_indices = _secondary_var.dofIndices();
      jacobian_types = {JType::SecondarySecondary, JType::SecondaryPrimary, JType::SecondaryLower};
      break;

    case MType::Primary:
      dof_indices = _primary_var.dofIndicesNeighbor();
      jacobian_types = {JType::PrimarySecondary, JType::PrimaryPrimary, JType::PrimaryLower};
      break;

    case MType::Lower:
      if (_var)
        dof_indices = _var->dofIndicesLower();
      jacobian_types = {JType::LowerSecondary, JType::LowerPrimary, JType::LowerLower};
      break;
  }
  test_space_size = dof_indices.size();

  residuals.resize(test_space_size, 0);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (_i = 0; _i < test_space_size; _i++)
      residuals[_i] += _JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type);

  auto local_functor = [&](const std::vector<ADReal> & input_residuals,
                           const std::vector<dof_id_type> &,
                           const std::set<TagID> &)
  {
    auto & ce = _assembly.couplingEntries();
    for (const auto & it : ce)
    {
      MooseVariableFEBase & ivariable = *(it.first);
      MooseVariableFEBase & jvariable = *(it.second);

      unsigned int ivar = ivariable.number();
      unsigned int jvar = jvariable.number();

      switch (mortar_type)
      {
        case MType::Secondary:
          if (ivar != _secondary_var.number())
            continue;
          break;

        case MType::Primary:
          if (ivar != _primary_var.number())
            continue;
          break;

        case MType::Lower:
          if (!_var || _var->number() != ivar)
            continue;
          break;
      }

      // Derivatives are offset by the variable number
      std::vector<size_t> ad_offsets{
          Moose::adOffset(jvar, _sys.getMaxVarNDofsPerElem(), Moose::ElementType::Element),
          Moose::adOffset(jvar,
                          _sys.getMaxVarNDofsPerElem(),
                          Moose::ElementType::Neighbor,
                          _sys.system().n_vars()),
          Moose::adOffset(jvar,
                          _sys.getMaxVarNDofsPerElem(),
                          Moose::ElementType::Lower,
                          _sys.system().n_vars())};
      std::vector<size_t> shape_space_sizes{jvariable.dofIndices().size(),
                                            jvariable.dofIndicesNeighbor().size(),
                                            jvariable.dofIndicesLower().size()};

      for (MooseIndex(3) type_index = 0; type_index < 3; ++type_index)
      {
        const auto jacobian_type = jacobian_types[type_index];
        // There's no actual coupling between secondary and primary dofs
        if ((jacobian_type == JType::SecondaryPrimary) ||
            (jacobian_type == JType::PrimarySecondary))
          continue;

        prepareMatrixTagLower(_assembly, ivar, jvar, jacobian_type);
        for (_i = 0; _i < test_space_size; _i++)
          for (_j = 0; _j < shape_space_sizes[type_index]; _j++)
          {
#ifndef MOOSE_SPARSE_AD
            mooseAssert(ad_offsets[type_index] + _j < MOOSE_AD_MAX_DOFS_PER_ELEM,
                        "Out of bounds access in derivative vector.");
#endif
            _local_ke(_i, _j) += input_residuals[_i].derivatives()[ad_offsets[type_index] + _j];
          }
        accumulateTaggedLocalMatrix();
      }
    }
  };

  _assembly.processDerivatives(residuals, dof_indices, _matrix_tags, local_functor);
}

#ifdef MOOSE_GLOBAL_AD_INDEXING
void
ADMortarConstraint::trimDerivative(const dof_id_type & remove_derivative_index, ADReal & var)
{
  auto md_it = var.derivatives().nude_data().begin();
  auto mi_it = var.derivatives().nude_indices().begin();

  auto d_it = var.derivatives().nude_data().begin();

  for (auto i_it = var.derivatives().nude_indices().begin();
       i_it != var.derivatives().nude_indices().end();
       ++i_it, ++d_it)
    if (*i_it != remove_derivative_index)
    {
      *mi_it = *i_it;
      *md_it = *d_it;
      ++mi_it;
      ++md_it;
    }

  std::size_t n_indices = md_it - var.derivatives().nude_data().begin();
  var.derivatives().nude_indices().resize(n_indices);
  var.derivatives().nude_data().resize(n_indices);
}

void
ADMortarConstraint::trimInteriorNodeDerivatives(
    const std::map<unsigned int, unsigned int> & domain_ip_lowerd_map,
    const std::vector<const MooseVariable *> & moose_vars,
    std::vector<ADReal *> & ad_vars,
    const bool is_secondary)
{
  // Remove interior node variable's derivatives from AD objects.
  for (const auto dof_index :
       (is_secondary ? make_range(_test_secondary.size()) : make_range(_test_primary.size())))
    if (!domain_ip_lowerd_map.count(dof_index))
    {
      for (const auto moose_var_index : make_range(moose_vars.size()))
      {
        mooseAssert(moose_vars[moose_var_index]->isNodal(),
                    "Trimming of interior node's derivatives is only supported for Lagrange "
                    "elements in mortar constraints");

        const auto & remove_derivative_index_x =
            is_secondary ? moose_vars[moose_var_index]->dofIndices()[dof_index]
                         : moose_vars[moose_var_index]->dofIndicesNeighbor()[dof_index];
        trimDerivative(remove_derivative_index_x, *ad_vars[moose_var_index]);
      }
    }
}
#endif
