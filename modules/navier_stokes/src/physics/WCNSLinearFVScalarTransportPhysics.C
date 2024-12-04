//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVScalarTransportPhysics.h"
#include "WCNSFVFlowPhysicsBase.h"
#include "NS.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSLinearFVScalarTransportPhysics);
registerWCNSFVScalarTransportBaseTasks("NavierStokesApp", WCNSLinearFVScalarTransportPhysics);

InputParameters
WCNSLinearFVScalarTransportPhysics::validParams()
{
  InputParameters params = WCNSFVScalarTransportPhysicsBase::validParams();
  params.addClassDescription("Define the Navier Stokes weakly-compressible scalar field transport "
                             "equation(s) using the linear finite volume discretization");
  params.addParam<bool>("use_nonorthogonal_correction",
                        true,
                        "If the nonorthogonal correction should be used when computing the normal "
                        "gradient, notably in the diffusion term.");
  return params;
}

WCNSLinearFVScalarTransportPhysics::WCNSLinearFVScalarTransportPhysics(
    const InputParameters & parameters)
  : WCNSFVScalarTransportPhysicsBase(parameters)
{
}

void
WCNSLinearFVScalarTransportPhysics::addSolverVariables()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  auto params = getFactory().getValidParams("MooseLinearVariableFVReal");
  assignBlocks(params, _blocks);

  for (const auto name_i : index_range(_passive_scalar_names))
  {
    // Dont add if the user already defined the variable
    if (variableExists(_passive_scalar_names[name_i], /*error_if_aux=*/true))
    {
      checkBlockRestrictionIdentical(
          _passive_scalar_names[name_i],
          getProblem().getVariable(0, _passive_scalar_names[name_i]).blocks());
      continue;
    }

    params.set<SolverSystemName>("solver_sys") = getSolverSystem(name_i);
    if (isParamValid("passive_scalar_scaling"))
      params.set<std::vector<Real>>("scaling") = {
          getParam<std::vector<Real>>("passive_scalar_scaling")[name_i]};

    getProblem().addVariable("MooseLinearVariableFVReal", _passive_scalar_names[name_i], params);
  }
}

void
WCNSLinearFVScalarTransportPhysics::addScalarTimeKernels()
{
  paramError("transient", "Transient simulations are not supported at this time");
}

void
WCNSLinearFVScalarTransportPhysics::addScalarAdvectionKernels()
{
  const std::string kernel_type = "LinearFVScalarAdvection";
  InputParameters params = getFactory().getValidParams(kernel_type);

  assignBlocks(params, _blocks);
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("passive_scalar_advection_interpolation");
  setSlipVelocityParams(params);

  for (const auto & vname : _passive_scalar_names)
  {
    params.set<LinearVariableName>("variable") = vname;
    getProblem().addLinearFVKernel(kernel_type, prefix() + "ins_" + vname + "_advection", params);
  }
}

void
WCNSLinearFVScalarTransportPhysics::addScalarDiffusionKernels()
{
  // Direct specification of diffusion term
  const auto passive_scalar_diffusivities =
      getParam<std::vector<MooseFunctorName>>("passive_scalar_diffusivity");

  if (passive_scalar_diffusivities.size())
  {
    const std::string kernel_type = "LinearFVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<bool>("use_nonorthogonal_correction") =
        getParam<bool>("use_nonorthogonal_correction");
    for (const auto name_i : index_range(_passive_scalar_names))
    {
      params.set<LinearVariableName>("variable") = _passive_scalar_names[name_i];
      params.set<MooseFunctorName>("diffusion_coeff") = passive_scalar_diffusivities[name_i];
      getProblem().addLinearFVKernel(
          kernel_type, prefix() + "ins_" + _passive_scalar_names[name_i] + "_diffusion", params);
    }
  }
}

void
WCNSLinearFVScalarTransportPhysics::addScalarSourceKernels()
{
  const std::string kernel_type = "LinearFVSource";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  for (const auto scalar_i : index_range(_passive_scalar_names))
  {
    params.set<LinearVariableName>("variable") = _passive_scalar_names[scalar_i];

    if (_passive_scalar_sources.size())
    {
      // Added for backward compatibility with former Modules/NavierStokesFV syntax
      params.set<MooseFunctorName>("source_density") = _passive_scalar_sources[scalar_i];
      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_" + _passive_scalar_names[scalar_i] + "_source", params);
    }

    if (_passive_scalar_coupled_sources.size())
      for (const auto i : index_range(_passive_scalar_coupled_sources[scalar_i]))
      {
        params.set<MooseFunctorName>("source_density") =
            _passive_scalar_coupled_sources[scalar_i][i];
        if (_passive_scalar_sources_coef.size())
          params.set<Real>("scaling_factor") = _passive_scalar_sources_coef[scalar_i][i];

        getProblem().addLinearFVKernel(kernel_type,
                                       prefix() + "ins_" + _passive_scalar_names[scalar_i] +
                                           "_coupled_source_" + std::to_string(i),
                                       params);
      }
  }
}

void
WCNSLinearFVScalarTransportPhysics::addScalarInletBC()
{
  const auto & inlet_boundaries = _flow_equations_physics->getInletBoundaries();
  if (inlet_boundaries.empty())
    return;

  // Boundary checks
  // TODO: once we have vectors of MooseEnum, we could use the same templated check for types and
  // functors
  if (inlet_boundaries.size() * _passive_scalar_names.size() != _passive_scalar_inlet_types.size())
    paramError(
        "passive_scalar_inlet_types",
        "The number of scalar inlet types (" + std::to_string(_passive_scalar_inlet_types.size()) +
            ") is not equal to the number of inlet boundaries (" +
            std::to_string(inlet_boundaries.size()) + ") times the number of passive scalars (" +
            std::to_string(_passive_scalar_names.size()) + ")");
  if (_passive_scalar_names.size() != _passive_scalar_inlet_functors.size())
    paramError("passive_scalar_inlet_functors",
               "The number of groups of inlet functors (" +
                   std::to_string(_passive_scalar_inlet_functors.size()) +
                   ") is not equal to the number of passive scalars (" +
                   std::to_string(_passive_scalar_names.size()) + ")");

  for (const auto name_i : index_range(_passive_scalar_names))
  {
    if (inlet_boundaries.size() != _passive_scalar_inlet_functors[name_i].size())
      paramError("passive_scalar_inlet_functors",
                 "The number of inlet boundary functors for scalar '" +
                     _passive_scalar_names[name_i] +
                     "' does not match the number of inlet boundaries (" +
                     std::to_string(_passive_scalar_inlet_functors[name_i].size()) + ")");

    unsigned int num_inlets = inlet_boundaries.size();
    for (unsigned int bc_ind = 0; bc_ind < num_inlets; ++bc_ind)
    {
      if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "fixed-value")
      {
        const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<LinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<MooseFunctorName>("functor") = _passive_scalar_inlet_functors[name_i][bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

        getProblem().addLinearFVBC(
            bc_type, _passive_scalar_names[name_i] + "_" + inlet_boundaries[bc_ind], params);
      }
      else if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass" ||
               _passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-velocity")
      {
        mooseError("Flux boundary conditions not supported at this time using the linear finite "
                   "volume discretization");
      }
    }
  }
}

void
WCNSLinearFVScalarTransportPhysics::addScalarOutletBC()
{
  const auto & outlet_boundaries = _flow_equations_physics->getOutletBoundaries();
  if (outlet_boundaries.empty())
    return;

  for (const auto & outlet_bdy : outlet_boundaries)
  {
    const std::string bc_type = "LinearFVAdvectionDiffusionOutflowBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
    params.set<bool>("use_two_term_expansion") =
        getParam<bool>("passive_scalar_two_term_bc_expansion");

    for (const auto name_i : index_range(_passive_scalar_names))
    {
      params.set<LinearVariableName>("variable") = _passive_scalar_names[name_i];
      getProblem().addLinearFVBC(bc_type, _passive_scalar_names[name_i] + "_" + outlet_bdy, params);
    }
  }
}
