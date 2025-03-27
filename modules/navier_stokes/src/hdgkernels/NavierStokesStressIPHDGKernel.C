//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesStressIPHDGAssemblyHelper.h"
#include "NavierStokesStressIPHDGKernel.h"

registerMooseObject("NavierStokesApp", NavierStokesStressIPHDGKernel);

InputParameters
NavierStokesStressIPHDGKernel::validParams()
{
  auto params = IPHDGKernel::validParams();
  params += NavierStokesStressIPHDGAssemblyHelper::validParams();
  params.addClassDescription(
      "Adds viscous and pressure stress terms for element interiors and interior faces");
  return params;
}

NavierStokesStressIPHDGKernel::NavierStokesStressIPHDGKernel(const InputParameters & params)
  : IPHDGKernel(params),
    _iphdg_helper(std::make_unique<NavierStokesStressIPHDGAssemblyHelper>(
        this, this, this, _mesh, _sys, _assembly, _tid, blockIDs(), std::set<BoundaryID>{}))
{
}
