//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionIPHDGAssemblyHelper.h"
#include "AdvectionIPHDGPrescribedFluxBC.h"

registerMooseObject("NavierStokesApp", AdvectionIPHDGPrescribedFluxBC);

InputParameters
AdvectionIPHDGPrescribedFluxBC::validParams()
{
  auto params = IPHDGPrescribedFluxBC::validParams();
  params += AdvectionIPHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements a prescribed flux condition for use with a hybridized "
                             "discretization of the advection equation");
  return params;
}

AdvectionIPHDGPrescribedFluxBC::AdvectionIPHDGPrescribedFluxBC(const InputParameters & parameters)
  : IPHDGPrescribedFluxBC(parameters),
    _iphdg_helper(std::make_unique<AdvectionIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs()))
{
}
