//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionIPHDGAssemblyHelper.h"
#include "DiffusionIPHDGPrescribedGradientBC.h"

registerMooseObject("MooseApp", DiffusionIPHDGPrescribedGradientBC);

InputParameters
DiffusionIPHDGPrescribedGradientBC::validParams()
{
  auto params = IPHDGPrescribedFluxBC::validParams();
  params += DiffusionIPHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements a flux boundary condition for use with a hybridized "
                             "discretization of the diffusion equation");
  return params;
}

DiffusionIPHDGPrescribedGradientBC::DiffusionIPHDGPrescribedGradientBC(
    const InputParameters & parameters)
  : IPHDGPrescribedFluxBC(parameters),
    _iphdg_helper(std::make_unique<DiffusionIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs()))
{
}
