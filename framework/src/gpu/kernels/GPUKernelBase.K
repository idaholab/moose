//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUKernelBase.h"

#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"

InputParameters
GPUKernelBase::validParams()
{
  auto params = GPUResidualObject::validParams();
  params += BlockRestrictable::validParams();
  params += MaterialPropertyInterface::validParams();

  // Kernels always couple within their element
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 0; });
  return params;
}

GPUKernelBase::GPUKernelBase(const InputParameters & parameters, Moose::VarFieldType field_type)
  : GPUResidualObject(parameters, field_type),
    BlockRestrictable(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS)
{
}

GPUKernelBase::GPUKernelBase(const GPUKernelBase & object)
  : GPUResidualObject(object),
    BlockRestrictable(object),
    CoupleableMooseVariableDependencyIntermediateInterface(&object, false, false, false),
    MaterialPropertyInterface(&object, blockIDs(), Moose::EMPTY_BOUNDARY_IDS, false)
{
}

void
GPUKernelBase::setVariableDependency()
{
  auto needed_moose_vars = getMooseVariableDependencies();

  for (auto & system : systems())
    if (system.initialized())
      system.setActiveVariables(needed_moose_vars);

  systems().copy();
}

void
GPUKernelBase::setProjectionFlags()
{
  for (auto & system : systems())
    if (system.initialized())
      system.setProjectionFlags(blockIDs());
}
