//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionBase.h"

namespace Moose
{

InputParameters
FunctionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();

  // Functions should be executed on the fly
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

FunctionBase::FunctionBase(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    UserObjectInterface(this),
    Restartable(this, "Functions"),
    MeshChangedInterface(parameters),
    ScalarCoupleable(this)
{
}

#ifdef MOOSE_KOKKOS_ENABLED
FunctionBase::FunctionBase(const FunctionBase & object, const Moose::Kokkos::FunctorCopy & key)
  : MooseObject(object, key),
    SetupInterface(object, key),
    TransientInterface(object, key),
    PostprocessorInterface(object, key),
    UserObjectInterface(object, key),
    Restartable(object, key),
    MeshChangedInterface(object, key),
    ScalarCoupleable(object, key)
{
}
#endif

FunctionBase::~FunctionBase() {}

} // namespace Moose
