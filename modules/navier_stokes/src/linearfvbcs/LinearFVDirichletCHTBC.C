//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVDirichletCHTBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVDirichletCHTBC);

InputParameters
LinearFVDirichletCHTBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorDirichletBC::validParams();
  params.addClassDescription(
      "Conjugate heat transfer BC for Dirichlet boundary condition-based coupling.");
  return params;
}

LinearFVDirichletCHTBC::LinearFVDirichletCHTBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorDirichletBC(parameters), LinearFVCHTBCInterface()
{
}
