//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluxKernelBase.h"

#include "MooseVariableFV.h"
#include "SystemBase.h"
#include "FVDirichletBC.h"
#include "MooseMesh.h"
#include "ADUtils.h"
#include "libmesh/elem.h"

InputParameters
FVFluxKernelBase::validParams()
{
  InputParameters params = FVKernel::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.registerSystemAttributeName("FVFluxKernel");
  params.addParam<bool>("force_boundary_execution",
                        false,
                        "Whether to force execution of this object on the boundary.");
  return params;
}

FVFluxKernelBase::FVFluxKernelBase(const InputParameters & params)
  : FVKernel(params),
    TwoMaterialPropertyInterface(this, blockIDs(), {}),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(
        this, false, false, /*is_fv=*/true),
    _force_boundary_execution(getParam<bool>("force_boundary_execution"))
{
}

bool
FVFluxKernelBase::skipForBoundary(const FaceInfo & fi)
{
  if (!fi.isBoundary() || _force_boundary_execution)
    return false;

  // If we have flux bcs then we do skip
  const auto & flux_pr = fieldVar().getFluxBCs(fi);
  if (flux_pr.first)
    return true;

  // If we don't have flux bcs *and* we do have dirichlet bcs then we don't skip. If we don't have
  // either then we assume natural boundary condition and we should skip
  return !fieldVar().hasDirichletBC(fi);
}
