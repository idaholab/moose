//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideIndicator.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

template <typename ComputeValueType>
InputParameters
InternalSideIndicatorTempl<ComputeValueType>::validParams()
{
  InputParameters params = InternalSideIndicatorBase::validParams();
  params.addRequiredParam<VariableName>(
      "variable", "The name of the variable that this side indicator applies to");

  return params;
}

template <typename ComputeValueType>
InternalSideIndicatorTempl<ComputeValueType>::InternalSideIndicatorTempl(
    const InputParameters & parameters)
  : InternalSideIndicatorBase(parameters),
    NeighborMooseVariableInterface<ComputeValueType>(this,
                                                     false,
                                                     Moose::VarKindType::VAR_ANY,
                                                     std::is_same<Real, ComputeValueType>::value
                                                         ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                         : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(this->mooseVariableField()),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_neighbor(_var.slnNeighbor()),
    _grad_u_neighbor(_var.gradSlnNeighbor())
{
  addMooseVariableDependency(&this->mooseVariableField());
}

// Explicitly instantiate three versions of the InternalSideIndicatorTempl class
template class InternalSideIndicatorTempl<Real>;
template class InternalSideIndicatorTempl<RealVectorValue>;
template class InternalSideIndicatorTempl<RealEigenVector>;
