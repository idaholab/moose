//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "WCNSLinearFVFlowPhysics.h"

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp", WCNSLinearFVFlowPhysics);
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_linear_fv_bc");
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_functor_material");

InputParameters
WCNSLinearFVFlowPhysics::validParams()
{
  InputParameters params = WCNSLinearFVFlowPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible equations with the linear "
      "solver implementation of the SIMPLE scheme");
  return params;
}

WCNSLinearFVFlowPhysics::WCNSLinearFVFlowPhysics(const InputParameters & parameters)
  : WCNSLinearFVFlowPhysicsBase(parameters)
{
}
