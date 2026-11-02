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
registerMooseAction("MooseTestApp", SharedVariablePhysics, "add_fv_kernel");
registerPhysicsBaseTasks("MooseTestApp", SharedVariablePhysics);

InputParameters
SharedVariablePhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Test Physics that requests variables shared with other instances.");
  params.addParam<bool>("fv", false, "Whether the shared solver variable is finite volume");
  params.addParam<std::string>(
      "order", "FIRST", "Order of the shared solver variable (finite element only)");
  params.addParam<Real>("scaling", 1, "Scaling factor of the shared solver variable");
  return params;
}

SharedVariablePhysics::SharedVariablePhysics(const InputParameters & parameters)
  : PhysicsBase(parameters), _fv(getParam<bool>("fv")), _order(getParam<std::string>("order"))
{
}

InputParameters
SharedVariablePhysics::getAdditionalRMParams() const
{
  if (!_fv)
    return emptyInputParameters();
  // An object with the ghosting needs of the finite volume diffusion kernel
  return getFactory().getValidParams("FVDiffusion");
}

void
SharedVariablePhysics::addSolverVariables()
{
  if (_fv)
  {
    const std::string var_type = "MooseVariableFVReal";
    auto params = getFactory().getValidParams(var_type);
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("scaling")};
    addSharedSolverVariable(var_type, "u", params);
  }
  else
    addSharedSolverVariable("u", "LAGRANGE", _order, getParam<Real>("scaling"));
}

void
SharedVariablePhysics::addAuxiliaryVariables()
{
  addSharedAuxVariable("v", "MONOMIAL", "CONSTANT");
}

void
SharedVariablePhysics::addFEKernels()
{
  if (_fv)
    return;
  // Only possible if the shared variable exists by the time kernels are added
  auto params = getFactory().getValidParams("Diffusion");
  params.set<NonlinearVariableName>("variable") = "u";
  assignBlocks(params, _blocks);
  getProblem().addKernel("Diffusion", name() + "_diffusion", params);
}

void
SharedVariablePhysics::addFVKernels()
{
  if (!_fv)
    return;
  auto params = getFactory().getValidParams("FVDiffusion");
  params.set<NonlinearVariableName>("variable") = "u";
  params.set<MooseFunctorName>("coeff") = "1";
  assignBlocks(params, _blocks);
  getProblem().addFVKernel("FVDiffusion", name() + "_diffusion", params);
}
