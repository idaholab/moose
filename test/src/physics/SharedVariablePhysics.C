//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SharedVariablePhysics.h"

registerMooseAction("MooseTestApp", SharedVariablePhysics, "add_variables_physics");
registerMooseAction("MooseTestApp", SharedVariablePhysics, "add_aux_variable");
registerMooseAction("MooseTestApp", SharedVariablePhysics, "add_kernel");
registerPhysicsBaseTasks("MooseTestApp", SharedVariablePhysics);

InputParameters
SharedVariablePhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Test Physics that requests variables shared with other instances.");
  params.addParam<Real>("scaling", 1, "Scaling factor of the shared solver variable");
  return params;
}

SharedVariablePhysics::SharedVariablePhysics(const InputParameters & parameters)
  : PhysicsBase(parameters)
{
}

void
SharedVariablePhysics::addSolverVariables()
{
  addSharedSolverVariable("u", "LAGRANGE", "FIRST", getParam<Real>("scaling"));
}

void
SharedVariablePhysics::addAuxiliaryVariables()
{
  addSharedAuxVariable("v", "MONOMIAL", "CONSTANT");
}

void
SharedVariablePhysics::addFEKernels()
{
  // Only possible if the shared variable exists by the time kernels are added
  auto params = getFactory().getValidParams("Diffusion");
  params.set<NonlinearVariableName>("variable") = "u";
  assignBlocks(params, _blocks);
  getProblem().addKernel("Diffusion", name() + "_diffusion", params);
}
