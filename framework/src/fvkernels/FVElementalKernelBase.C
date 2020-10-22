//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVElementalKernelBase.h"
#include "MooseVariableFV.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "NonlinearSystemBase.h"
#include "ADUtils.h"

#include "libmesh/elem.h"

#include "metaphysicl/raw_type.h"

InputParameters
FVElementalKernelBase::validParams()
{
  InputParameters params = FVKernel::validParams();
  params.registerSystemAttributeName("FVElementalKernel");
  params += MaterialPropertyInterface::validParams();
  return params;
}

FVElementalKernelBase::FVElementalKernelBase(const InputParameters & parameters)
  : FVKernel(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false, /*is_fv=*/true),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    _current_elem(_assembly.elem()),
    _q_point(_assembly.qPoints())
{
}
