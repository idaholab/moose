//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayDGLowerDKernel.h"

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
ArrayDGLowerDKernel::validParams()
{
  InputParameters params = ArrayDGKernel::validParams();
  params.addRequiredCoupledVar("lowerd_variable", "Lagrange multiplier");
  return params;
}

ArrayDGLowerDKernel::ArrayDGLowerDKernel(const InputParameters & parameters)
  : ArrayDGKernel(parameters),
    _lowerd_var(*getArrayVar("lowerd_variable", 0)),
    _lambda(_is_implicit ? _lowerd_var.slnLower() : _lowerd_var.slnLowerOld()),

    _phi_lambda(_lowerd_var.phiLower()),
    _test_lambda(_lowerd_var.phiLower()),
    _work_vector(_count)
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
                 "Must not be defined on the interior lower-dimensional subdomain '" +
                     _mesh.getSubdomainName(id) + "'");

  if (_lowerd_var.count() != _count)
    paramError("lowerd_variable",
               "The number of components must be equal to the number of "
               "components of 'variable'");
}

void
ArrayDGLowerDKernel::computeResidual()
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
ArrayDGLowerDKernel::computeLowerDResidual()
{
  prepareVectorTagLower(_assembly, _lowerd_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initLowerDQpResidual();
    for (_i = 0; _i < _test_lambda.size(); _i++)
    {
      _work_vector.setZero();
      computeLowerDQpResidual(_work_vector);
      mooseAssert(_work_vector.size() == _count,
                  "Size of local residual is not equal to the number of array variable compoments");
      _work_vector *= _JxW[_qp] * _coord[_qp];
      _assembly.saveLocalArrayResidual(_local_re, _i, _test_lambda.size(), _work_vector);
    }
  }

  accumulateTaggedLocalResidual();
}

void
ArrayDGLowerDKernel::computeJacobian()
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
ArrayDGLowerDKernel::computeLowerDJacobian(Moose::ConstraintJacobianType type)
{
  mooseAssert(type != Moose::PrimaryPrimary && type != Moose::PrimarySecondary &&
                  type != Moose::SecondaryPrimary && type != Moose::SecondarySecondary,
              "Jacobian types without lower should be handled in computeElemNeighJacobian");

  const ArrayVariableTestValue & test_space =
      (type == Moose::LowerLower || type == Moose::LowerSecondary || type == Moose::LowerPrimary)
          ? _test_lambda
          : (type == Moose::PrimaryLower ? _test : _test_neighbor);
  unsigned int ivar =
      (type == Moose::LowerLower || type == Moose::LowerSecondary || type == Moose::LowerPrimary)
          ? _lowerd_var.number()
          : _var.number();

  const ArrayVariableTestValue & loc_phi =
      (type == Moose::LowerLower || type == Moose::SecondaryLower || type == Moose::PrimaryLower)
          ? _phi_lambda
          : (type == Moose::LowerPrimary ? _phi : _phi_neighbor);
  unsigned int jvar =
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
      {
        // vector or matrix?
        RealEigenVector v = _JxW[_qp] * _coord[_qp] * computeLowerDQpJacobian(type);
        _assembly.saveDiagLocalArrayJacobian(
            _local_ke, _i, test_space.size(), _j, loc_phi.size(), ivar, v);
      }
  }

  accumulateTaggedLocalMatrix();
}

void
ArrayDGLowerDKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (!excludeBoundary())
  {
    precalculateOffDiagJacobian(jvar_num);

    /*
     * Note: we cannot call compute jacobian functions like in DGLowerDKernel
     *       because we could have cross component couplings for the array variables
     */

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

void
ArrayDGLowerDKernel::computeOffDiagLowerDJacobian(Moose::ConstraintJacobianType type,
                                                  const MooseVariableFEBase & jvar)
{
  mooseAssert(type != Moose::PrimaryPrimary && type != Moose::PrimarySecondary &&
                  type != Moose::SecondaryPrimary && type != Moose::SecondarySecondary,
              "Jacobian types without lower should be handled in computeOffDiagElemNeighJacobian");

  const ArrayVariableTestValue & test_space =
      (type == Moose::LowerLower || type == Moose::LowerSecondary || type == Moose::LowerPrimary)
          ? _test_lambda
          : (type == Moose::PrimaryLower ? _test : _test_neighbor);
  unsigned int ivar =
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
        {
          RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeLowerDQpOffDiagJacobian(type, jvar);
          _assembly.saveFullLocalArrayJacobian(
              _local_ke, _i, test_space.size(), _j, loc_phi.size(), ivar, jvar.number(), v);
        }
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    const auto & jv1 = static_cast<const ArrayMooseVariable &>(jvar);
    const ArrayVariableTestValue & loc_phi =
        (type == Moose::LowerLower || type == Moose::SecondaryLower || type == Moose::PrimaryLower)
            ? jv1.phiLower()
            : (type == Moose::LowerPrimary ? jv1.phiFace() : jv1.phiFaceNeighbor());

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initLowerDQpOffDiagJacobian(type, jvar);
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
        {
          RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeLowerDQpOffDiagJacobian(type, jvar);
          _assembly.saveFullLocalArrayJacobian(
              _local_ke, _i, test_space.size(), _j, loc_phi.size(), ivar, jvar.number(), v);
        }
    }
  }
  else
    mooseError("Vector variable cannot be coupled into array DG kernel currently");

  accumulateTaggedLocalMatrix();
}

RealEigenMatrix
ArrayDGLowerDKernel::computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                    const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
  {
    if (type == Moose::LowerSecondary || type == Moose::LowerPrimary)
      return computeLowerDQpJacobian(type).asDiagonal();
  }
  else if (jvar.number() == _lowerd_var.number())
  {
    if (type == Moose::SecondaryLower || type == Moose::PrimaryLower || type == Moose::LowerLower)
      return computeLowerDQpJacobian(type).asDiagonal();
  }

  return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
