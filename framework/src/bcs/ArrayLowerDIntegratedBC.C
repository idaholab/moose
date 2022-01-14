//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayLowerDIntegratedBC.h"

#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

InputParameters
ArrayLowerDIntegratedBC::validParams()
{
  InputParameters params = ArrayIntegratedBC::validParams();
  params.addRequiredCoupledVar("lowerd_variable", "Lagrange multiplier");
  return params;
}

ArrayLowerDIntegratedBC::ArrayLowerDIntegratedBC(const InputParameters & parameters)
  : ArrayIntegratedBC(parameters),
    _lowerd_var(*getArrayVar("lowerd_variable", 0)),
    _lambda(_is_implicit ? _lowerd_var.slnLower() : _lowerd_var.slnLowerOld()),

    _phi_lambda(_lowerd_var.phiLower()),
    _test_lambda(_lowerd_var.phiLower()),
    _work_vector(_count)
{
  const auto & lower_domains = _lowerd_var.activeSubdomains();
  if (!lower_domains.count(Moose::BOUNDARY_SIDE_LOWERD_ID) && lower_domains.size() != 1)
    paramError(
        "lowerd_variable",
        "Must be only defined on the subdomain BOUNDARY_SIDE_LOWERD_SUBDOMAIN subdomain that is "
        "added by Mesh/build_all_side_lowerd_mesh=true");

  if (_var.activeSubdomains().count(Moose::BOUNDARY_SIDE_LOWERD_ID))
    paramError("variable",
               "Must not be defined on the subdomain BOUNDARY_SIDE_LOWERD_SUBDOMAIN subdomain");

  if (_lowerd_var.count() != _count)
    paramError("lowerd_variable",
               "The number of components must be equal to the number of "
               "components of 'variable'");
}

void
ArrayLowerDIntegratedBC::computeResidual()
{
  ArrayIntegratedBC::computeResidual();

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
ArrayLowerDIntegratedBC::computeJacobian()
{
  ArrayIntegratedBC::computeJacobian();

  computeLowerDJacobian(Moose::LowerLower);
}

void
ArrayLowerDIntegratedBC::computeLowerDJacobian(Moose::ConstraintJacobianType type)
{
  mooseAssert(type == Moose::LowerLower || type == Moose::LowerPrimary ||
                  type == Moose::PrimaryLower,
              "Jacobian types must have lower in computeLowerDJacobian");

  const ArrayVariableTestValue & test_space =
      (type == Moose::LowerLower || type == Moose::LowerPrimary) ? _test_lambda : _test;
  unsigned int ivar = (type == Moose::LowerLower || type == Moose::LowerPrimary)
                          ? _lowerd_var.number()
                          : _var.number();

  const ArrayVariableTestValue & loc_phi =
      (type == Moose::LowerLower || type == Moose::PrimaryLower) ? _phi_lambda : _phi;
  unsigned int jvar = (type == Moose::LowerLower || type == Moose::PrimaryLower)
                          ? _lowerd_var.number()
                          : _var.number();

  // need to transform the type for assembling Jacobian on boundary to be consistent with
  // Assembly::addJacobianLowerD() and Assembly::prepareLowerD().
  Moose::ConstraintJacobianType type_transformed =
      (type == Moose::LowerLower
           ? type
           : (type == Moose::LowerPrimary ? Moose::LowerSecondary : Moose::SecondaryLower));
  prepareMatrixTagLower(_assembly, ivar, jvar, type_transformed);

  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initLowerDQpJacobian(type);
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
      {
        RealEigenVector v = _JxW[_qp] * _coord[_qp] * computeLowerDQpJacobian(type);
        _assembly.saveDiagLocalArrayJacobian(
            _local_ke, _i, test_space.size(), _j, loc_phi.size(), ivar, v);
      }
  }

  accumulateTaggedLocalMatrix();
}

void
ArrayLowerDIntegratedBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  ArrayIntegratedBC::computeOffDiagJacobian(jvar_num);

  computeLowerDOffDiagJacobian(Moose::LowerLower, jvar_num);
  computeLowerDOffDiagJacobian(Moose::LowerPrimary, jvar_num);
  computeLowerDOffDiagJacobian(Moose::PrimaryLower, jvar_num);
}

void
ArrayLowerDIntegratedBC::computeLowerDOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                      const unsigned int jvar_num)
{
  mooseAssert(type == Moose::LowerLower || type == Moose::LowerPrimary ||
                  type == Moose::PrimaryLower,
              "Jacobian types must have lower in computeLowerDJacobian");

  const ArrayVariableTestValue & test_space =
      (type == Moose::LowerLower || type == Moose::LowerPrimary) ? _test_lambda : _test;
  unsigned int ivar = (type == Moose::LowerLower || type == Moose::LowerPrimary)
                          ? _lowerd_var.number()
                          : _var.number();

  const auto & jvar = getVariable(jvar_num);

  // need to transform the type for assembling Jacobian on boundary to be consistent with
  // Assembly::addJacobianLowerD() and Assembly::prepareLowerD().
  Moose::ConstraintJacobianType type_transformed =
      (type == Moose::LowerLower
           ? type
           : (type == Moose::LowerPrimary ? Moose::LowerSecondary : Moose::SecondaryLower));
  prepareMatrixTagLower(_assembly, ivar, jvar_num, type_transformed);
  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    const auto & jv0 = static_cast<const MooseVariable &>(jvar);
    const VariableTestValue & loc_phi =
        (type == Moose::LowerLower || type == Moose::PrimaryLower) ? jv0.phiLower() : jv0.phiFace();

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initLowerDQpOffDiagJacobian(type, jvar);
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
        {
          RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeLowerDQpOffDiagJacobian(type, jvar);
          _assembly.saveFullLocalArrayJacobian(
              _local_ke, _i, test_space.size(), _j, loc_phi.size(), ivar, jvar_num, v);
        }
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    const auto & jv1 = static_cast<const ArrayMooseVariable &>(jvar);
    const ArrayVariableTestValue & loc_phi =
        (type == Moose::LowerLower || type == Moose::PrimaryLower) ? jv1.phiLower() : jv1.phiFace();

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initLowerDQpOffDiagJacobian(type, jvar);
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
        {
          RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeLowerDQpOffDiagJacobian(type, jvar);
          _assembly.saveFullLocalArrayJacobian(
              _local_ke, _i, test_space.size(), _j, loc_phi.size(), ivar, jvar_num, v);
        }
    }
  }
  else
    mooseError("Vector variable cannot be coupled into array DG kernel currently");

  accumulateTaggedLocalMatrix();
}

RealEigenMatrix
ArrayLowerDIntegratedBC::computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                        const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
  {
    if (type == Moose::LowerPrimary)
      return computeLowerDQpJacobian(type).asDiagonal();
  }
  else if (jvar.number() == _lowerd_var.number())
  {
    if (type == Moose::PrimaryLower || type == Moose::LowerLower)
      return computeLowerDQpJacobian(type).asDiagonal();
  }

  return RealEigenMatrix::Zero(_count, jvar.count());
}
