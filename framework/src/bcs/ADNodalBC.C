//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNodalBC.h"

// MOOSE includes
#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

defineADBaseValidParams(ADNodalBC, NodalBCBase, );
defineADBaseValidParams(ADVectorNodalBC, NodalBCBase, );

template <typename T, ComputeStage compute_stage>
ADNodalBCTempl<T, compute_stage>::ADNodalBCTempl(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<T>(this,
                              true,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*this->mooseVariable()),
    _current_node(_var.node()),
    _u(_var.template adNodalValue<compute_stage>())
{
  addMooseVariableDependency(this->mooseVariable());
}

template <typename T>
T &
conversionHelper(T & value, const unsigned int &)
{
  return value;
}

template <typename T>
T &
conversionHelper(libMesh::VectorValue<T> & value, const unsigned int & i)
{
  return value(i);
}

template <typename T, ComputeStage compute_stage>
void
ADNodalBCTempl<T, compute_stage>::computeResidual()
{
  const std::vector<dof_id_type> & dof_indices = _var.dofIndices();

  auto residual = computeQpResidual();

  for (auto tag_id : _vector_tags)
    if (_sys.hasVector(tag_id))
      for (size_t i = 0; i < dof_indices.size(); ++i)
        _sys.getVector(tag_id).set(dof_indices[i], conversionHelper(residual, i));
}

template <>
void
ADNodalBCTempl<Real, JACOBIAN>::computeResidual()
{
}

template <>
void
ADNodalBCTempl<RealVectorValue, JACOBIAN>::computeResidual()
{
}

template <typename T, ComputeStage compute_stage>
void
ADNodalBCTempl<T, compute_stage>::computeJacobian()
{
  auto ad_offset = _var.number() * _sys.getMaxVarNDofsPerNode();
  auto residual = computeQpResidual();
  const std::vector<dof_id_type> & cached_rows = _var.dofIndices();

  // Cache the user's computeQpJacobian() value for later use.
  for (auto tag : _matrix_tags)
    if (_sys.hasMatrix(tag))
      for (size_t i = 0; i < cached_rows.size(); ++i)
        _fe_problem.assembly(0).cacheJacobianContribution(
            cached_rows[i],
            cached_rows[i],
            conversionHelper(residual, i).derivatives()[ad_offset + i],
            tag);
}

template <>
void
ADNodalBCTempl<Real, RESIDUAL>::computeJacobian()
{
}
template <>
void
ADNodalBCTempl<RealVectorValue, RESIDUAL>::computeJacobian()
{
}

template <typename T, ComputeStage compute_stage>
void
ADNodalBCTempl<T, compute_stage>::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    auto ad_offset = jvar * _sys.getMaxVarNDofsPerNode();
    auto residual = computeQpResidual();
    const std::vector<dof_id_type> & cached_rows = _var.dofIndices();

    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

    // Cache the user's computeQpJacobian() value for later use.
    for (auto tag : _matrix_tags)
      if (_sys.hasMatrix(tag))
        for (size_t i = 0; i < cached_rows.size(); ++i)
          _fe_problem.assembly(0).cacheJacobianContribution(
              cached_rows[i],
              cached_col,
              conversionHelper(residual, i).derivatives()[ad_offset + i],
              tag);
  }
}

template <>
void
ADNodalBCTempl<Real, RESIDUAL>::computeOffDiagJacobian(unsigned int)
{
}
template <>
void
ADNodalBCTempl<RealVectorValue, RESIDUAL>::computeOffDiagJacobian(unsigned int)
{
}

template <typename T, ComputeStage compute_stage>
void
ADNodalBCTempl<T, compute_stage>::computeOffDiagJacobianScalar(unsigned int jvar)
{
  auto ad_offset = jvar * _sys.getMaxVarNDofsPerNode();
  auto residual = computeQpResidual();
  const std::vector<dof_id_type> & cached_rows = _var.dofIndices();

  std::vector<dof_id_type> scalar_dof_indices;

  _sys.dofMap().SCALAR_dof_indices(scalar_dof_indices, jvar);

  // Our residuals rely on returning a single scalar and we don't provide any arguments to
  // computeQpResidual so I think it only makes sense to assume that our SCALAR variable should be
  // order one
  mooseAssert(scalar_dof_indices.size() == 1,
              "ADNodalBC only allows coupling of first order SCALAR variables");

  // Cache the user's computeQpJacobian() value for later use.
  for (auto tag : _matrix_tags)
    if (_sys.hasMatrix(tag))
      for (size_t i = 0; i < cached_rows.size(); ++i)
        _fe_problem.assembly(0).cacheJacobianContribution(
            cached_rows[i],
            scalar_dof_indices[0],
            conversionHelper(residual, i).derivatives()[ad_offset + i],
            tag);
}

template <>
void
ADNodalBCTempl<Real, RESIDUAL>::computeOffDiagJacobianScalar(unsigned int)
{
}

template <>
void
ADNodalBCTempl<RealVectorValue, RESIDUAL>::computeOffDiagJacobianScalar(unsigned int)
{
}

template class ADNodalBCTempl<Real, RESIDUAL>;
template class ADNodalBCTempl<Real, JACOBIAN>;
template class ADNodalBCTempl<RealVectorValue, RESIDUAL>;
template class ADNodalBCTempl<RealVectorValue, JACOBIAN>;
