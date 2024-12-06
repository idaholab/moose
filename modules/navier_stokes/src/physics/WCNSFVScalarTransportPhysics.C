//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVScalarTransportPhysics.h"
#include "WCNSFVFlowPhysicsBase.h"
#include "NSFVBase.h"
#include "NS.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSFVScalarTransportPhysics);
registerWCNSFVScalarTransportBaseTasks("NavierStokesApp", WCNSFVScalarTransportPhysics);

InputParameters
WCNSFVScalarTransportPhysics::validParams()
{
  InputParameters params = WCNSFVScalarTransportPhysicsBase::validParams();
  params.addClassDescription("Define the Navier Stokes weakly-compressible scalar field transport "
                             "equation(s) using the nonlinear finite volume discretization");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "passive_scalar_face_interpolation");
  params.addParamNamesToGroup("passive_scalar_face_interpolation", "Numerical scheme");
  return params;
}

WCNSFVScalarTransportPhysics::WCNSFVScalarTransportPhysics(const InputParameters & parameters)
  : WCNSFVScalarTransportPhysicsBase(parameters)
{
}

void
WCNSFVScalarTransportPhysics::addSolverVariables()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  auto params = getFactory().getValidParams("INSFVScalarFieldVariable");
  assignBlocks(params, _blocks);
  params.set<MooseEnum>("face_interp_method") =
      getParam<MooseEnum>("passive_scalar_face_interpolation");
  params.set<bool>("two_term_boundary_expansion") =
      getParam<bool>("passive_scalar_two_term_bc_expansion");

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

    getProblem().addVariable("INSFVScalarFieldVariable", _passive_scalar_names[name_i], params);
  }
}

void
WCNSFVScalarTransportPhysics::addScalarTimeKernels()
{
  for (const auto & vname : _passive_scalar_names)
  {
    const std::string kernel_type = "FVFunctorTimeKernel";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = vname;

    getProblem().addFVKernel(kernel_type, prefix() + "ins_" + vname + "_time", params);
  }
}

void
WCNSFVScalarTransportPhysics::addScalarAdvectionKernels()
{
  const std::string kernel_type = "INSFVScalarFieldAdvection";
  InputParameters params = getFactory().getValidParams(kernel_type);

  assignBlocks(params, _blocks);
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("passive_scalar_advection_interpolation");
  setSlipVelocityParams(params);

  for (const auto & vname : _passive_scalar_names)
  {
    params.set<NonlinearVariableName>("variable") = vname;
    getProblem().addFVKernel(kernel_type, prefix() + "ins_" + vname + "_advection", params);
  }
}

void
WCNSFVScalarTransportPhysics::addScalarDiffusionKernels()
{
  // Direct specification of diffusion term
  const auto passive_scalar_diffusivities =
      getParam<std::vector<MooseFunctorName>>("passive_scalar_diffusivity");

  if (passive_scalar_diffusivities.size())
  {
    const std::string kernel_type = "FVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    for (const auto name_i : index_range(_passive_scalar_names))
    {
      params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
      params.set<MooseFunctorName>("coeff") = passive_scalar_diffusivities[name_i];
      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_" + _passive_scalar_names[name_i] + "_diffusion", params);
    }
  }
}

void
WCNSFVScalarTransportPhysics::addScalarSourceKernels()
{
  const std::string kernel_type = "FVCoupledForce";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  for (const auto scalar_i : index_range(_passive_scalar_names))
  {
    params.set<NonlinearVariableName>("variable") = _passive_scalar_names[scalar_i];
    if (_passive_scalar_sources.size())
    {
      // Added for backward compatibility with former Modules/NavierStokesFV syntax
      params.set<MooseFunctorName>("v") = _passive_scalar_sources[scalar_i];
      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_" + _passive_scalar_names[scalar_i] + "_source", params);
    }

    // Sufficient for all intents and purposes
    if (_passive_scalar_coupled_sources.size())
      for (const auto i : index_range(_passive_scalar_coupled_sources[scalar_i]))
      {
        params.set<MooseFunctorName>("v") = _passive_scalar_coupled_sources[scalar_i][i];
        if (_passive_scalar_sources_coef.size())
          params.set<Real>("coef") = _passive_scalar_sources_coef[scalar_i][i];

        getProblem().addFVKernel(kernel_type,
                                 prefix() + "ins_" + _passive_scalar_names[scalar_i] +
                                     "_coupled_source_" + std::to_string(i),
                                 params);
      }
  }
}

void
WCNSFVScalarTransportPhysics::addScalarInletBC()
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

    unsigned int flux_bc_counter = 0;
    unsigned int num_inlets = inlet_boundaries.size();
    for (unsigned int bc_ind = 0; bc_ind < num_inlets; ++bc_ind)
    {
      if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "fixed-value")
      {
        const std::string bc_type = "FVFunctionDirichletBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<FunctionName>("function") = _passive_scalar_inlet_functors[name_i][bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

        getProblem().addFVBC(
            bc_type, _passive_scalar_names[name_i] + "_" + inlet_boundaries[bc_ind], params);
      }
      else if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass" ||
               _passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-velocity")
      {
        const auto flux_inlet_directions = _flow_equations_physics->getFluxInletDirections();
        const auto flux_inlet_pps = _flow_equations_physics->getFluxInletPPs();

        const std::string bc_type = "WCNSFVScalarFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<MooseFunctorName>("passive_scalar") = _passive_scalar_names[name_i];
        if (flux_inlet_directions.size())
          params.set<Point>("direction") = flux_inlet_directions[flux_bc_counter];
        if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + inlet_boundaries[bc_ind];
        }
        else
          params.set<PostprocessorName>("velocity_pp") = flux_inlet_pps[flux_bc_counter];

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<PostprocessorName>("scalar_value_pp") =
            _passive_scalar_inlet_functors[name_i][bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

        params.set<MooseFunctorName>(NS::velocity_x) = _velocity_names[0];
        if (dimension() > 1)
          params.set<MooseFunctorName>(NS::velocity_y) = _velocity_names[1];
        if (dimension() > 2)
          params.set<MooseFunctorName>(NS::velocity_z) = _velocity_names[2];

        getProblem().addFVBC(bc_type,
                             prefix() + _passive_scalar_names[name_i] + "_" +
                                 inlet_boundaries[bc_ind],
                             params);
        flux_bc_counter += 1;
      }
    }
  }
}

void
WCNSFVScalarTransportPhysics::addScalarOutletBC()
{
  // Advection outlet is naturally handled by the advection flux kernel
  return;
}
