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
  return NodalBCBase::validParams();
}

template <>
InputParameters
ADNodalBCTempl<RealVectorValue>::validParams()
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
        {std::is_same<T, RealVectorValue>::value ? getParam<bool>("set_x_comp") : true,
         std::is_same<T, RealVectorValue>::value ? getParam<bool>("set_y_comp") : true,
         std::is_same<T, RealVectorValue>::value ? getParam<bool>("set_z_comp") : true}),
    _undisplaced_assembly(_fe_problem.assembly(_tid, _sys.number()))
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
template <typename ADResidual>
void
ADNodalBCTempl<T>::processResidual(const ADResidual & residual,
                                   const std::vector<dof_id_type> & dof_indices)
{
  mooseAssert(dof_indices.size() <= _set_components.size(),
              "The number of dof indices must be less than the number of settable components");

  for (auto tag_id : _vector_tags)
    if (_sys.hasVector(tag_id))
      for (const auto i : index_range(dof_indices))
        if (_set_components[i])
          _sys.getVector(tag_id).set(dof_indices[i], raw_value(conversionHelper(residual, i)));
}

template <typename T>
template <typename ADResidual>
void
ADNodalBCTempl<T>::processJacobian(const ADResidual & residual,
                                   const std::vector<dof_id_type> & dof_indices)
{
  mooseAssert(dof_indices.size() <= _set_components.size(),
              "The number of dof indices must be less than the number of settable components");

  for (const auto i : index_range(dof_indices))
    if (_set_components[i])
      // If we store into the displaced assembly for nodal bc objects the data never actually makes
      // it into the global Jacobian
      _undisplaced_assembly.processJacobianNoScaling(
          conversionHelper(residual, i), dof_indices[i], _matrix_tags);
}

template <typename T>
void
ADNodalBCTempl<T>::computeResidual()
{
  const std::vector<dof_id_type> & dof_indices = _var.dofIndices();
  if (dof_indices.empty())
    return;

  const auto residual = computeQpResidual();

  processResidual(residual, dof_indices);
}

template <typename T>
void
ADNodalBCTempl<T>::computeJacobian()
{
  const std::vector<dof_id_type> & dof_indices = _var.dofIndices();
  if (dof_indices.empty())
    return;

  const auto residual = computeQpResidual();

  processJacobian(residual, dof_indices);
}

template <typename T>
void
ADNodalBCTempl<T>::computeResidualAndJacobian()
{
  const std::vector<dof_id_type> & dof_indices = _var.dofIndices();
  if (dof_indices.empty())
    return;

  const auto residual = computeQpResidual();

  processResidual(residual, dof_indices);
  processJacobian(residual, dof_indices);
}

template <typename T>
void
ADNodalBCTempl<T>::computeOffDiagJacobian(const unsigned int jvar_num)
{
  // Only need to do this once because AD does all the derivatives at once
  if (jvar_num == _var.number())
    computeJacobian();
}

template <typename T>
void
ADNodalBCTempl<T>::computeOffDiagJacobianScalar(unsigned int)
{
  // scalar coupling will have been included in the all-at-once handling in computeOffDiagJacobian
}

template class ADNodalBCTempl<Real>;
template class ADNodalBCTempl<RealVectorValue>;
