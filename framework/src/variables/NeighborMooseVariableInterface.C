//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NeighborMooseVariableInterface.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseTypes.h"
#include "Problem.h"
#include "SubProblem.h"

template <typename T>
NeighborMooseVariableInterface<T>::NeighborMooseVariableInterface(
    const MooseObject * moose_object,
    bool nodal,
    Moose::VarKindType expected_var_type,
    Moose::VarFieldType expected_var_field_type)
  : MooseVariableInterface<T>(
        moose_object, nodal, "variable", expected_var_type, expected_var_field_type)
{
}

template <typename T>
NeighborMooseVariableInterface<T>::~NeighborMooseVariableInterface()
{
}

template <typename T>
const typename OutputTools<T>::VariableValue &
NeighborMooseVariableInterface<T>::neighborValue()
{
  if (this->_nodal)
    return this->_variable->dofValuesNeighbor();
  else
    return this->_variable->slnNeighbor();
}

template <>
const VectorVariableValue &
NeighborMooseVariableInterface<RealVectorValue>::neighborValue()
{
  if (this->_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return this->_variable->slnNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableValue &
NeighborMooseVariableInterface<T>::neighborValueOld()
{
  if (this->_nodal)
    return this->_variable->dofValuesOldNeighbor();
  else
    return this->_variable->slnOldNeighbor();
}

template <>
const VectorVariableValue &
NeighborMooseVariableInterface<RealVectorValue>::neighborValueOld()
{
  if (this->_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return this->_variable->slnOldNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableValue &
NeighborMooseVariableInterface<T>::neighborValueOlder()
{
  if (this->_nodal)
    return this->_variable->dofValuesOlderNeighbor();
  else
    return this->_variable->slnOlderNeighbor();
}

template <>
const VectorVariableValue &
NeighborMooseVariableInterface<RealVectorValue>::neighborValueOlder()
{
  if (this->_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return this->_variable->slnOlderNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableGradient &
NeighborMooseVariableInterface<T>::neighborGradient()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have gradients");

  return this->_variable->gradSlnNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableGradient &
NeighborMooseVariableInterface<T>::neighborGradientOld()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have gradients");

  return this->_variable->gradSlnOldNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableGradient &
NeighborMooseVariableInterface<T>::neighborGradientOlder()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have gradients");

  return this->_variable->gradSlnOlderNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableSecond &
NeighborMooseVariableInterface<T>::neighborSecond()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return this->_variable->secondSlnNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableSecond &
NeighborMooseVariableInterface<T>::neighborSecondOld()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return this->_variable->secondSlnOldNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableSecond &
NeighborMooseVariableInterface<T>::neighborSecondOlder()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return this->_variable->secondSlnOlderNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariableTestSecond &
NeighborMooseVariableInterface<T>::neighborSecondTest()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return this->_variable->secondPhiFaceNeighbor();
}

template <typename T>
const typename OutputTools<T>::VariablePhiSecond &
NeighborMooseVariableInterface<T>::neighborSecondPhi()
{
  if (this->_nodal)
    mooseError("Nodal variables do not have second derivatives");

  return this->_mvi_assembly->secondPhiFaceNeighbor(*this->_variable);
}

template class NeighborMooseVariableInterface<Real>;
template class NeighborMooseVariableInterface<RealVectorValue>;
template class NeighborMooseVariableInterface<RealEigenVector>;
