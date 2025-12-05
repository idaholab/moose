//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalKernelBase.h"

namespace Moose::Kokkos
{

InputParameters
NodalKernelBase::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();

  params.registerBase("NodalKernel");

  return params;
}

NodalKernelBase::NodalKernelBase(const InputParameters & parameters, Moose::VarFieldType field_type)
  : ResidualObject(parameters, field_type),
    BlockRestrictable(this),
    BoundaryRestrictable(this, true), // true for applying to nodesets
    CoupleableMooseVariableDependencyIntermediateInterface(this, true)
{
}

NodalKernelBase::NodalKernelBase(const NodalKernelBase & object)
  : ResidualObject(object),
    BlockRestrictable(object, {}),
    BoundaryRestrictable(object, {}),
    CoupleableMooseVariableDependencyIntermediateInterface(object, {})
{
}

} // namespace Moose::Kokkos
