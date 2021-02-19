//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMomentumPressure.h"
#include "NS.h"

registerMooseObjectRenamed("NavierStokesApp",
                           INSFVMomentumPressure,
                           "07/01/2021 00:00",
                           FVMomentumPressure);
registerMooseObjectRenamed("NavierStokesApp",
                           CNSFVMomPressure,
                           "07/01/2021 00:00",
                           FVMomentumPressure);
registerMooseObject("NavierStokesApp", FVMomentumPressure);

InputParameters
FVMomentumPressure::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Introduces the coupled pressure term into the Navier-Stokes momentum equation.");
  params.addCoupledVar("p", "The pressure");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  // In the worst case scenario this pressure gradient will be computed by a material and if we are
  // doing FV then we will also have a neighbor material that is computing a cell center gradient.
  // And cell ceneter gradients need one layer of ghosting. So total layers = neighbor needs
  // gradient (1) + neighbor cell center gradient (1) = 2
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVMomentumPressure::FVMomentumPressure(const InputParameters & params)
  : FVElementalKernel(params),
    _grad_pressure(isCoupled("p")
                       ? adCoupledGradient("p")
                       : getADMaterialProperty<RealVectorValue>(NS::grad(NS::pressure)).get()),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
FVMomentumPressure::computeQpResidual()
{
  return _grad_pressure[_qp](_index);
}
