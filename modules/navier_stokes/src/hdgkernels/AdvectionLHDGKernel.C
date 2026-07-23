//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionLHDGAssemblyHelper.h"
#include "AdvectionLHDGKernel.h"

registerMooseObject("NavierStokesApp", AdvectionLHDGKernel);

InputParameters
AdvectionLHDGKernel::validParams()
{
  auto params = HDGKernel::validParams();
  params += AdvectionLHDGAssemblyHelper::validParams();
  params.addClassDescription("Adds element and interior face integrals for an L-HDG advection "
                             "term using cell and hybrid velocities, respectively.");
  return params;
}

AdvectionLHDGKernel::AdvectionLHDGKernel(const InputParameters & params)
  : HDGKernel(params),
    _lhdg_helper(std::make_unique<AdvectionLHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, blockIDs(), std::set<BoundaryID>{}))
{
}

HDGAssemblyHelper *
AdvectionLHDGKernel::hdgHelper()
{
  return _lhdg_helper.get();
}
