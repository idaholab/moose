//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMortarLagrangeConstraint.h"

// MOOSE includes
#include "MooseVariable.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "ADUtils.h"

InputParameters
ADMortarLagrangeConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  return params;
}

ADMortarLagrangeConstraint::ADMortarLagrangeConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters)
{
}

void
ADMortarLagrangeConstraint::computeJacobian(Moose::MortarType mortar_type)
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

  // If we don't interpolate geometry, only some residuals/dofs are populated.
  // Eliminating unnecessary residual component is necessary to preserve the right order of
  // derivative entries when performing the processing of derivatives in assembly.
  std::vector<DualReal> residuals_lower;
  std::vector<dof_id_type> dof_indices_lower;

  if (!_interpolate_normals)
  {
    for (_i = 0; _i < test_space_size; _i++)
    {
      const bool skip_dof =
          ((!_secondary_ip_lowerd_map.count(_i) && mortar_type == Moose::MortarType::Secondary)) ||
          ((!_primary_ip_lowerd_map.count(_i) && mortar_type == Moose::MortarType::Primary));

      if (!skip_dof)
      {
        residuals_lower.push_back(residuals[_i]);
        dof_indices_lower.push_back(dof_indices[_i]);
      }
    }
  }
  else
  {
    residuals_lower = residuals;
    dof_indices_lower = dof_indices;
  }

  auto local_functor = [&](const std::vector<ADReal> & input_residuals,
                           const std::vector<dof_id_type> &,
                           const std::set<TagID> &) {
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
        unsigned int i_lower = 0;

        for (_i = 0; _i < test_space_size; _i++)
        {
          // Do we need to skip the derivatives of a zero residual?
          const bool skip_dof =
              ((!_interpolate_normals && (!_secondary_ip_lowerd_map.count(_i) &&
                                          mortar_type == Moose::MortarType::Secondary)) ||
               (!_interpolate_normals &&
                (!_primary_ip_lowerd_map.count(_i) && mortar_type == Moose::MortarType::Primary)));

          if (skip_dof)
            continue;

          for (_j = 0; _j < shape_space_sizes[type_index]; _j++)
          {
#ifndef MOOSE_SPARSE_AD
            mooseAssert(ad_offsets[type_index] + _j < MOOSE_AD_MAX_DOFS_PER_ELEM,
                        "Out of bounds access in derivative vector.");
#endif
            _local_ke(_i, _j) +=
                input_residuals[i_lower].derivatives()[ad_offsets[type_index] + _j];
          }
          i_lower++;
        }
        accumulateTaggedLocalMatrix();
      }
    }
  };

  _assembly.processDerivatives(residuals_lower, dof_indices_lower, _matrix_tags, local_functor);
}
