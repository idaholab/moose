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

template <typename T>
InputParameters
ADNodalBCTempl<T>::validParams()
{
  InputParameters params = NodalBCBase::validParams();

  // The below parameters are useful for vector Nodal BCs
  params.addParam<bool>("set_x_comp", true, "Whether to set the x-component of the variable");
  params.addParam<bool>("set_y_comp", true, "Whether to set the y-component of the variable");
  params.addParam<bool>("set_z_comp", true, "Whether to set the z-component of the variable");
  return params;
}

template <typename T>
ADNodalBCTempl<T>::ADNodalBCTempl(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<T>(this,
                              true,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*this->mooseVariable()),
    _current_node(_var.node()),
    _u(_var.adNodalValue()),
    _set_components(
        {getParam<bool>("set_x_comp"), getParam<bool>("set_y_comp"), getParam<bool>("set_z_comp")})
{
  _subproblem.haveADObjects(true);

  addMooseVariableDependency(this->mooseVariable());
}

namespace
{
const ADReal &
conversionHelper(const ADReal & value, const unsigned int)
{
  return value;
}

const ADReal &
conversionHelper(const libMesh::VectorValue<ADReal> & value, const unsigned int i)
{
  return value(i);
}
}

template <typename T>
void
ADNodalBCTempl<T>::computeResidual()
{
  const std::vector<dof_id_type> & dof_indices = _var.dofIndices();
  if (dof_indices.empty())
    return;

  mooseAssert(dof_indices.size() <= _set_components.size(),
              "The number of dof indices must be less than the number of settable components");

  auto residual = computeQpResidual();

  for (auto tag_id : _vector_tags)
    if (_sys.hasVector(tag_id))
      for (std::size_t i = 0; i < dof_indices.size(); ++i)
        if (_set_components[i])
          _sys.getVector(tag_id).set(dof_indices[i], raw_value(conversionHelper(residual, i)));
}

template <typename T>
void
ADNodalBCTempl<T>::computeJacobian()
{
  const std::vector<dof_id_type> & cached_rows = _var.dofIndices();
  if (cached_rows.empty())
    return;

  auto residual = computeQpResidual();

  mooseAssert(cached_rows.size() <= _set_components.size(),
              "The number of dof indices must be less than the number of settable components");

  // Cache the user's computeQpJacobian() value for later use.
  for (auto tag : _matrix_tags)
    if (_sys.hasMatrix(tag))
      for (std::size_t i = 0; i < cached_rows.size(); ++i)
        if (_set_components[i])
        {
          _fe_problem.assembly(0, _sys.number())
              .cacheJacobian(cached_rows[i],
                             cached_rows[i],
                             conversionHelper(residual, i).derivatives()[cached_rows[i]],
                             tag);
        }
}

template <typename T>
void
ADNodalBCTempl<T>::computeResidualAndJacobian()
{
  const auto residual = computeQpResidual();
  const std::vector<dof_id_type> & cached_rows = _var.dofIndices();

  mooseAssert(cached_rows.size() <= _set_components.size(),
              "The number of dof indices must be less than the number of settable components");

  // First the residuals
  for (auto tag_id : _vector_tags)
    if (_sys.hasVector(tag_id))
      for (const auto i : index_range(cached_rows))
        if (_set_components[i])
          _sys.getVector(tag_id).set(cached_rows[i], raw_value(conversionHelper(residual, i)));

  // Cache the user's computeQpJacobian() value for later use.
  for (auto tag_id : _matrix_tags)
    if (_sys.hasMatrix(tag_id))
      for (const auto i : index_range(cached_rows))
        if (_set_components[i])
          _fe_problem.assembly(0, _sys.number())
              .cacheJacobian(cached_rows[i],
                             cached_rows[i],
                             conversionHelper(residual, i).derivatives()[cached_rows[i]],
                             tag_id);
}

template <typename T>
void
ADNodalBCTempl<T>::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    const std::vector<dof_id_type> & cached_rows = _var.dofIndices();
    if (cached_rows.empty())
      return;

    auto residual = computeQpResidual();

    mooseAssert(cached_rows.size() <= _set_components.size(),
                "The number of dof indices must be less than the number of settable components");

    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar_num, 0);

    // Cache the user's computeQpJacobian() value for later use.
    for (auto tag : _matrix_tags)
      if (_sys.hasMatrix(tag))
        for (std::size_t i = 0; i < cached_rows.size(); ++i)
          if (_set_components[i])
          {
            _fe_problem.assembly(0, _sys.number())
                .cacheJacobian(cached_rows[i],
                               cached_col,
                               conversionHelper(residual, i).derivatives()[cached_col],
                               tag);
          }
  }
}

template <typename T>
void
ADNodalBCTempl<T>::computeOffDiagJacobianScalar(unsigned int jvar)
{
  auto residual = computeQpResidual();
  const std::vector<dof_id_type> & cached_rows = _var.dofIndices();

  mooseAssert(cached_rows.size() <= _set_components.size(),
              "The number of dof indices must be less than the number of settable components");

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
      for (std::size_t i = 0; i < cached_rows.size(); ++i)
        if (_set_components[i])
        {
          _fe_problem.assembly(0, _sys.number())
              .cacheJacobian(cached_rows[i],
                             scalar_dof_indices[0],
                             conversionHelper(residual, i).derivatives()[scalar_dof_indices[0]],
                             tag);
        }
}

template class ADNodalBCTempl<Real>;
template class ADNodalBCTempl<RealVectorValue>;
