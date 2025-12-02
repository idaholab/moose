//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesStressIPHDGAssemblyHelper.h"
#include "NavierStokesStressIPHDGPrescribedFluxBC.h"

registerMooseObject("NavierStokesApp", NavierStokesStressIPHDGPrescribedFluxBC);

InputParameters
NavierStokesStressIPHDGPrescribedFluxBC::validParams()
{
  auto params = IPHDGPrescribedFluxBC::validParams();
  params += NavierStokesStressIPHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements a prescribed stress boundary condition for use with a "
                             "hybridized discretization of the Navier-Stokes equation");
  return params;
}

NavierStokesStressIPHDGPrescribedFluxBC::NavierStokesStressIPHDGPrescribedFluxBC(
    const InputParameters & parameters)
  : IPHDGPrescribedFluxBC(parameters),
    _iphdg_helper(std::make_unique<NavierStokesStressIPHDGAssemblyHelper>(
        this, this, this, _mesh, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs()))
{
}
