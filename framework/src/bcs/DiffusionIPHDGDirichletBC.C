//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionIPHDGDirichletBC.h"
#include "DiffusionIPHDGAssemblyHelper.h"

registerMooseObject("MooseApp", DiffusionIPHDGDirichletBC);

InputParameters
DiffusionIPHDGDirichletBC::validParams()
{
  auto params = IPHDGDirichletBC::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of a diffusion equation");
  params += DiffusionIPHDGAssemblyHelper::validParams();
  return params;
}

DiffusionIPHDGDirichletBC::DiffusionIPHDGDirichletBC(const InputParameters & parameters)
  : IPHDGDirichletBC(parameters),
    _iphdg_helper(std::make_unique<DiffusionIPHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs()))
{
}
