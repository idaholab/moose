//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableField.h"
#include "SystemBase.h"
#include "TimeIntegrator.h"

#include "libmesh/fe_base.h"

using namespace Moose;

template <typename OutputType>
InputParameters
MooseVariableField<OutputType>::validParams()
{
  return MooseVariableFieldBase::validParams();
}

template <typename OutputType>
MooseVariableField<OutputType>::MooseVariableField(const InputParameters & parameters)
  : MooseVariableFieldBase(parameters),
    Moose::FunctorBase<typename Moose::ADType<OutputType>::type>(name()),
    MeshChangedInterface(parameters),
    _time_integrator(_sys.queryTimeIntegrator(_var_num))
{
}

template <typename OutputType>
void
MooseVariableField<OutputType>::residualSetup()
{
  MooseVariableFieldBase::residualSetup();
  FunctorBase<typename Moose::ADType<OutputType>::type>::residualSetup();
}

template <typename OutputType>
void
MooseVariableField<OutputType>::jacobianSetup()
{
  MooseVariableFieldBase::jacobianSetup();
  FunctorBase<typename Moose::ADType<OutputType>::type>::jacobianSetup();
}

template <typename OutputType>
void
MooseVariableField<OutputType>::timestepSetup()
{
  MooseVariableFieldBase::timestepSetup();
  FunctorBase<typename Moose::ADType<OutputType>::type>::timestepSetup();
}

template <typename OutputType>
Moose::VarFieldType
MooseVariableField<OutputType>::fieldType() const
{
  if (std::is_same<OutputType, Real>::value)
    return Moose::VarFieldType::VAR_FIELD_STANDARD;
  else if (std::is_same<OutputType, RealVectorValue>::value)
    return Moose::VarFieldType::VAR_FIELD_VECTOR;
  else if (std::is_same<OutputType, RealEigenVector>::value)
    return Moose::VarFieldType::VAR_FIELD_ARRAY;
  else
    mooseError("Unknown variable field type");
}

template <typename OutputType>
bool
MooseVariableField<OutputType>::isArray() const
{
  const auto is_array = MooseVariableBase::isArray();
  if (std::is_same<OutputType, RealEigenVector>::value != is_array)
    mooseError("A variable is marked as an array variable in a base class, but in a derived class "
               "the output type is not consistent.");
  return is_array;
}

template <typename OutputType>
bool
MooseVariableField<OutputType>::isVector() const
{
  return std::is_same<OutputType, RealVectorValue>::value;
}

template <>
template <>
const MooseArray<Real> &
MooseVariableField<Real>::genericDofValues<false>() const
{
  return dofValues();
}

template <>
template <>
const MooseArray<Real> &
MooseVariableField<RealVectorValue>::genericDofValues<false>() const
{
  return dofValues();
}

template <>
template <>
const MooseArray<Real> &
MooseVariableField<RealEigenVector>::genericDofValues<false>() const
{
  mooseError("genericDofValues not implemented for array variables");
}

template class MooseVariableField<Real>;
template class MooseVariableField<RealVectorValue>;
template class MooseVariableField<RealEigenVector>;
