//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesStressIPHDGDirichletBC.h"
#include "NavierStokesStressIPHDGAssemblyHelper.h"

registerMooseObject("MooseApp", NavierStokesStressIPHDGDirichletBC);

InputParameters
NavierStokesStressIPHDGDirichletBC::validParams()
{
  auto params = IPHDGDirichletBC::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of a Navier-Stokes equation stress term");
  params += NavierStokesStressIPHDGAssemblyHelper::validParams();
  return params;
}

NavierStokesStressIPHDGDirichletBC::NavierStokesStressIPHDGDirichletBC(
    const InputParameters & parameters)
  : IPHDGDirichletBC(parameters),
    _iphdg_helper(std::make_unique<NavierStokesStressIPHDGAssemblyHelper>(
        this, this, this, _mesh, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs()))
{
}
