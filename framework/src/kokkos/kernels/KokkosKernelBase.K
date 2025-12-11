//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosKernelBase.h"

#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"

namespace Moose::Kokkos
{

InputParameters
KernelBase::validParams()
{
  auto params = ResidualObject::validParams();
  params += BlockRestrictable::validParams();
  params += MaterialPropertyInterface::validParams();

  // Kernels always couple within their element
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 0; });

  params.registerBase("Kernel");

  return params;
}

KernelBase::KernelBase(const InputParameters & parameters, Moose::VarFieldType field_type)
  : ResidualObject(parameters, field_type),
    BlockRestrictable(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS)
{
}

KernelBase::KernelBase(const KernelBase & object)
  : ResidualObject(object),
    BlockRestrictable(object, {}),
    CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    MaterialPropertyInterface(object, {})
{
}

} // namespace Moose::Kokkos
