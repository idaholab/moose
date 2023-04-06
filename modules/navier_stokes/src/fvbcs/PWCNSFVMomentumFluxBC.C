//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PWCNSFVMomentumFluxBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PWCNSFVMomentumFluxBC);

InputParameters
PWCNSFVMomentumFluxBC::validParams()
{
  InputParameters params = WCNSFVMomentumFluxBC::validParams();
  params.addClassDescription("Flux boundary conditions for porous momentum advection.");
  params.addParam<MooseFunctorName>(NS::porosity, NS::porosity, "Porosity functor");
  return params;
}

PWCNSFVMomentumFluxBC::PWCNSFVMomentumFluxBC(const InputParameters & params)
  : WCNSFVMomentumFluxBC(params), _eps(getFunctor<ADReal>(NS::porosity))
{
}

ADReal
PWCNSFVMomentumFluxBC::computeQpResidual()
{
  return WCNSFVMomentumFluxBC::computeQpResidual() / _eps(singleSidedFaceArg(), determineState());
}
