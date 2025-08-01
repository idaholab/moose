//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMortarLagrangeConstraint.h"

// MOOSE includes
#include "MooseVariable.h"
#include "MoosePreconditioner.h"
#include "VariableCondensationPreconditioner.h"
#include "NonlinearSystemBase.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "ADUtils.h"
#include "FEProblemBase.h"
#include "AutomaticMortarGeneration.h"

#include <algorithm>
#include <vector>

InputParameters
ADMortarLagrangeConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addParam<Real>(
      "derivative_threshold",
      1.0e-17,
      "Threshold to discard automatic differentiation derivatives. This number is chosen on the "
      "order of the machine epsilon based on current experience.");

  return params;
}

ADMortarLagrangeConstraint::ADMortarLagrangeConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _ad_derivative_threshold(getParam<Real>("derivative_threshold")),
    _apply_derivative_threshold(true)
{
}

void
ADMortarLagrangeConstraint::initialSetup()
{
  SetupInterface::initialSetup();

  // Detect if preconditioner is VCP. If so, disable automatic derivative trimming.
  auto const * mpc = feProblem().getNonlinearSystemBase(_sys.number()).getPreconditioner();

  if (dynamic_cast<const VariableCondensationPreconditioner *>(mpc))
    _apply_derivative_threshold = false;
}

void
ADMortarLagrangeConstraint::computeResidual(Moose::MortarType mortar_type)
{
  _primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);

  _secondary_ip_lowerd_map = amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

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

  std::vector<unsigned int> is_index_on_lower_dimension;
  // Find out if nodes are on the surface
  for (_i = 0; _i < test_space_size; _i++)
  {
    if (mortar_type == Moose::MortarType::Primary && !_primary_ip_lowerd_map.count(_i))
      continue;

    if (mortar_type == Moose::MortarType::Secondary && !_secondary_ip_lowerd_map.count(_i))
      continue;

    is_index_on_lower_dimension.push_back(_i);
  }

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (const auto index : is_index_on_lower_dimension)
    {
      _i = index;
      _local_re(_i) += raw_value(_JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type));
    }

  accumulateTaggedLocalResidual();
}

void
ADMortarLagrangeConstraint::computeJacobian(Moose::MortarType mortar_type)
{
  std::vector<ADReal> residuals;
  size_t test_space_size = 0;
  typedef Moose::ConstraintJacobianType JType;
  typedef Moose::MortarType MType;
  std::vector<JType> jacobian_types;
  std::vector<dof_id_type> dof_indices;
  Real scaling_factor = 1;

  _primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);

  _secondary_ip_lowerd_map = amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  switch (mortar_type)
  {
    case MType::Secondary:
      dof_indices = _secondary_var.dofIndices();
      jacobian_types = {JType::SecondarySecondary, JType::SecondaryPrimary, JType::SecondaryLower};
      scaling_factor = _secondary_var.scalingFactor();
      break;

    case MType::Primary:
      dof_indices = _primary_var.dofIndicesNeighbor();
      jacobian_types = {JType::PrimarySecondary, JType::PrimaryPrimary, JType::PrimaryLower};
      scaling_factor = _primary_var.scalingFactor();
      break;

    case MType::Lower:
      mooseAssert(_var, "This should be non-null");
      dof_indices = _var->dofIndicesLower();
      jacobian_types = {JType::LowerSecondary, JType::LowerPrimary, JType::LowerLower};
      scaling_factor = _var->scalingFactor();
      break;
  }
  test_space_size = dof_indices.size();

  residuals.resize(test_space_size, 0);

  std::vector<unsigned int> is_index_on_lower_dimension;
  std::vector<dof_id_type> dof_indices_lower;

  // Find out if nodes are on the surface
  unsigned int number_indices_on_lowerd = 0;

  for (_i = 0; _i < test_space_size; _i++)
  {
    if (mortar_type == Moose::MortarType::Primary && !_primary_ip_lowerd_map.count(_i))
      continue;

    if (mortar_type == Moose::MortarType::Secondary && !_secondary_ip_lowerd_map.count(_i))
      continue;

    is_index_on_lower_dimension.push_back(_i);
    dof_indices_lower.push_back(dof_indices[_i]);
    number_indices_on_lowerd++;
  }

  std::vector<ADReal> residuals_lower;
  residuals_lower.resize(number_indices_on_lowerd, 0);

  // Only populate nodal residuals on the primary/secondary surfaces
  // We do this regardless of whether we are interpolating normals. Use of this class
  // implies we have Lagrange elements, so internal (high-dimensional) normals have no meaning
  // and should be zero. As such, we decide to omit them and avoid possible spurious population of
  // automatic differentiation-generated derivatives.
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    unsigned int index_lower = 0;
    for (const auto index : is_index_on_lower_dimension)
    {
      _i = index;
      residuals_lower[index_lower] += _JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type);

      // Get rid of derivatives that we assume won't count (tolerance prescribed by user)
      // This can cause zero diagonal terms with the variable condensation preconditioner when the
      // no adaptivity option is used (dofs are not checked).
      // Uncomment when https://github.com/libMesh/MetaPhysicL/pull/18 makes it to MOOSE

      index_lower++;
    }
  }

  addJacobian(_assembly, residuals_lower, dof_indices_lower, scaling_factor);
}
