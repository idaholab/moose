//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGLowerDKernel.h"

#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

InputParameters
DGLowerDKernel::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addRequiredCoupledVar("lowerd_variable", "Lagrange multiplier");
  return params;
}

DGLowerDKernel::DGLowerDKernel(const InputParameters & parameters)
  : DGKernel(parameters),
    _lowerd_var(*getVar("lowerd_variable", 0)),
    _lambda(_is_implicit ? _lowerd_var.slnLower() : _lowerd_var.slnLowerOld()),

    _phi_lambda(_lowerd_var.phiLower()),
    _test_lambda(_lowerd_var.phiLower())
{
  const auto & lower_domains = _lowerd_var.activeSubdomains();
  for (const auto & id : _mesh.interiorLowerDBlocks())
    if (lower_domains.count(id) == 0)
      mooseDocumentedError(
          "moose",
          29151,
          "'lowerd_variable' must be defined on the interior lower-dimensional subdomain '" +
              _mesh.getSubdomainName(id) +
              "' that is added by Mesh/build_all_side_lowerd_mesh=true.\nThe check could be overly "
              "restrictive.");

  for (const auto & id : _var.activeSubdomains())
    if (_mesh.interiorLowerDBlocks().count(id) > 0)
      paramError("variable",
                 "Must not be defined on the interior lower-dimensional subdomain'" +
                     _mesh.getSubdomainName(id) + "'");

  // Note: the above two conditions also ensure that the variable and lower-d variable are
  // different.
}

void
DGLowerDKernel::computeResidual()
{
  if (!excludeBoundary())
  {
    precalculateResidual();

    // Compute the residual for this element
    computeElemNeighResidual(Moose::Element);

    // Compute the residual for the neighbor
    computeElemNeighResidual(Moose::Neighbor);

    computeLowerDResidual();
  }
}

void
DGLowerDKernel::computeLowerDResidual()
{
  prepareVectorTagLower(_assembly, _lowerd_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initLowerDQpResidual();
    for (_i = 0; _i < _test_lambda.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeLowerDQpResidual();
  }

  accumulateTaggedLocalResidual();
}

void
DGLowerDKernel::computeJacobian()
{
  if (!excludeBoundary())
  {
    precalculateJacobian();

    // Compute element-element Jacobian
    computeElemNeighJacobian(Moose::ElementElement);

    // Compute element-neighbor Jacobian
    computeElemNeighJacobian(Moose::ElementNeighbor);

    // Compute neighbor-element Jacobian
    computeElemNeighJacobian(Moose::NeighborElement);

    // Compute neighbor-neighbor Jacobian
    computeElemNeighJacobian(Moose::NeighborNeighbor);

    // Compute the other five pieces of Jacobian related with lower-d variable
    computeLowerDJacobian(Moose::LowerLower);
  }
}

void
DGLowerDKernel::computeLowerDJacobian(Moose::ConstraintJacobianType type)
{
  mooseAssert(type != Moose::PrimaryPrimary && type != Moose::PrimarySecondary &&
                  type != Moose::SecondaryPrimary && type != Moose::SecondarySecondary,
              "Jacobian types without lower should be handled in computeElemNeighJacobian");

  const auto & test_space =
      (type == Moose::LowerLower || type == Moose::LowerSecondary || type == Moose::LowerPrimary)
          ? _test_lambda
          : (type == Moose::PrimaryLower ? _test : _test_neighbor);
  const auto ivar =
      (type == Moose::LowerLower || type == Moose::LowerSecondary || type == Moose::LowerPrimary)
          ? _lowerd_var.number()
          : _var.number();

  const auto & loc_phi =
      (type == Moose::LowerLower || type == Moose::SecondaryLower || type == Moose::PrimaryLower)
          ? _phi_lambda
          : (type == Moose::LowerPrimary ? _phi : _phi_neighbor);
  const auto jvar =
      (type == Moose::LowerLower || type == Moose::SecondaryLower || type == Moose::PrimaryLower)
          ? _lowerd_var.number()
          : _var.number();

  prepareMatrixTagLower(_assembly, ivar, jvar, type);

  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initLowerDQpJacobian(type);
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeLowerDQpJacobian(type);
  }

  accumulateTaggedLocalMatrix();
}

void
DGLowerDKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (!excludeBoundary())
  {
    precalculateOffDiagJacobian(jvar_num);

    if (jvar_num == variable().number())
    {
      computeElemNeighJacobian(Moose::ElementElement);
      computeElemNeighJacobian(Moose::ElementNeighbor);
      computeElemNeighJacobian(Moose::NeighborElement);
      computeElemNeighJacobian(Moose::NeighborNeighbor);
      computeLowerDJacobian(Moose::LowerSecondary);
      computeLowerDJacobian(Moose::LowerPrimary);
    }
    else if (jvar_num == _lowerd_var.number())
    {
      computeLowerDJacobian(Moose::LowerLower);
      computeLowerDJacobian(Moose::SecondaryLower);
      computeLowerDJacobian(Moose::PrimaryLower);
    }
    else
    {
      const auto & jvar = getVariable(jvar_num);

      // Compute element-element Jacobian
      computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);

      // Compute element-neighbor Jacobian
      computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);

      // Compute neighbor-element Jacobian
      computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);

      // Compute neighbor-neighbor Jacobian
      computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);

      // Compute the other five pieces of Jacobian related with lower-d variable
      computeOffDiagLowerDJacobian(Moose::LowerLower, jvar);
      computeOffDiagLowerDJacobian(Moose::LowerSecondary, jvar);
      computeOffDiagLowerDJacobian(Moose::LowerPrimary, jvar);
      computeOffDiagLowerDJacobian(Moose::SecondaryLower, jvar);
      computeOffDiagLowerDJacobian(Moose::PrimaryLower, jvar);
    }
  }
}

void
DGLowerDKernel::computeOffDiagLowerDJacobian(Moose::ConstraintJacobianType type,
                                             const MooseVariableFEBase & jvar)
{
  mooseAssert(type != Moose::PrimaryPrimary && type != Moose::PrimarySecondary &&
                  type != Moose::SecondaryPrimary && type != Moose::SecondarySecondary,
              "Jacobian types without lower should be handled in computeOffDiagElemNeighJacobian");

  const auto & test_space =
      (type == Moose::LowerLower || type == Moose::LowerSecondary || type == Moose::LowerPrimary)
          ? _test_lambda
          : (type == Moose::PrimaryLower ? _test : _test_neighbor);
  const auto ivar =
      (type == Moose::LowerLower || type == Moose::LowerSecondary || type == Moose::LowerPrimary)
          ? _lowerd_var.number()
          : _var.number();

  prepareMatrixTagLower(_assembly, ivar, jvar.number(), type);
  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    const auto & jv0 = static_cast<const MooseVariable &>(jvar);
    const VariableTestValue & loc_phi =
        (type == Moose::LowerLower || type == Moose::SecondaryLower || type == Moose::PrimaryLower)
            ? jv0.phiLower()
            : (type == Moose::LowerPrimary ? jv0.phiFace() : jv0.phiFaceNeighbor());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initLowerDQpOffDiagJacobian(type, jvar);
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeLowerDQpOffDiagJacobian(type, jvar);
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
    mooseError("Array variable cannot be coupled into DG kernel currently");
  else
    mooseError("Vector variable cannot be coupled into DG kernel currently");

  accumulateTaggedLocalMatrix();
}
