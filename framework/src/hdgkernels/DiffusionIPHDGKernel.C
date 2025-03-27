//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionIPHDGAssemblyHelper.h"
#include "DiffusionIPHDGKernel.h"

registerMooseObject("MooseApp", DiffusionIPHDGKernel);

InputParameters
DiffusionIPHDGKernel::validParams()
{
  auto params = IPHDGKernel::validParams();
  params += DiffusionIPHDGAssemblyHelper::validParams();
  params.addClassDescription(
      "Adds the element and interior face weak forms for a hybridized interior penalty "
      "discontinuous Galerkin discretization of a diffusion term.");
  return params;
}

DiffusionIPHDGKernel::DiffusionIPHDGKernel(const InputParameters & params)
  : IPHDGKernel(params),
    _iphdg_helper(std::make_unique<DiffusionIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, blockIDs(), std::set<BoundaryID>{}))
{
}
