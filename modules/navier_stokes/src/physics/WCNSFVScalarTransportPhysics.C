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
#include "MapConversionUtils.h"

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
  : PhysicsBase(parameters), WCNSFVScalarTransportPhysicsBase(parameters)
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

  // Check the size of the scalar sources inputs
  for (const auto scalar_i : index_range(_passive_scalar_names))
  {
    if (_passive_scalar_coupled_sources_blocks.size())
      if (_passive_scalar_coupled_sources_blocks.size() !=
          _passive_scalar_coupled_sources[scalar_i].size())
        paramError("passive_scalar_coupled_source",
                   "Number of coupled sources (" +
                       std::to_string(_passive_scalar_coupled_sources[scalar_i].size()) +
                       ") does not match the number of groups of blocks "
                       "specified for the coupled sources (" +
                       std::to_string(_passive_scalar_coupled_sources_blocks.size()) + ")");
    if (_passive_scalar_coupled_sources_coefs.size())
      if (_passive_scalar_coupled_sources_coefs[scalar_i].size() !=
          _passive_scalar_coupled_sources[scalar_i].size())
        paramError("passive_scalar_coupled_source_coeff",
                   "Number of coupled sources (" +
                       std::to_string(_passive_scalar_coupled_sources[scalar_i].size()) +
                       ") does not match the number of coupled sources coefficients (" +
                       std::to_string(_passive_scalar_coupled_sources_coefs[scalar_i].size()) +
                       ")");
  }

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
        if (_passive_scalar_coupled_sources_coefs.size())
          params.set<MooseFunctorName>("functor_coef") =
              _passive_scalar_coupled_sources_coefs[scalar_i][i];
        if (_passive_scalar_coupled_sources_blocks.size())
          params.set<std::vector<SubdomainName>>("block") =
              _passive_scalar_coupled_sources_blocks[i];

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
    for (const auto & boundary : inlet_boundaries)
    {
      const auto & boundary_type = libmesh_map_find(_passive_scalar_inlet_types[name_i], boundary);
      if (boundary_type == "fixed-value")
      {
        const std::string bc_type = "FVFunctionDirichletBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<FunctionName>("function") = _passive_scalar_inlet_functors[name_i][boundary];
        params.set<std::vector<BoundaryName>>("boundary") = {boundary};

        getProblem().addFVBC(bc_type, _passive_scalar_names[name_i] + "_" + boundary, params);
      }
      else if (boundary_type == "flux-mass" || boundary_type == "flux-velocity")
      {
        const auto flux_inlet_directions = _flow_equations_physics->getFluxInletDirections();
        const auto flux_inlet_pps = _flow_equations_physics->getFluxInletPPs();

        const std::string bc_type = "WCNSFVScalarFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<MooseFunctorName>("passive_scalar") = _passive_scalar_names[name_i];
        if (flux_inlet_directions.size())
          params.set<Point>("direction") = flux_inlet_directions[flux_bc_counter];
        if (boundary_type == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + boundary;
        }
        else
          params.set<PostprocessorName>("velocity_pp") = flux_inlet_pps[flux_bc_counter];

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<PostprocessorName>("scalar_value_pp") =
            _passive_scalar_inlet_functors[name_i][boundary];
        params.set<std::vector<BoundaryName>>("boundary") = {boundary};

        params.set<unsigned int>("dimension") = dimension();
        params.set<MooseFunctorName>(NS::velocity_x) = _velocity_names[0];
        if (dimension() > 1)
          params.set<MooseFunctorName>(NS::velocity_y) = _velocity_names[1];
        if (dimension() > 2)
          params.set<MooseFunctorName>(NS::velocity_z) = _velocity_names[2];

        getProblem().addFVBC(
            bc_type, prefix() + _passive_scalar_names[name_i] + "_" + boundary, params);
        flux_bc_counter += 1;
      }
    }
  }
}

void
WCNSFVScalarTransportPhysics::addInletBoundary(const BoundaryName & boundary,
                                               const MooseEnum & inlet_type,
                                               const MooseFunctorName & inlet_functor,
                                               const unsigned int scalar_index)
{
  _passive_scalar_inlet_types[scalar_index].insert(std::make_pair(boundary, inlet_type));
  if (inlet_type == "fixed-value" || inlet_type == "flux-mass" || inlet_type == "flux-velocity")
    _passive_scalar_inlet_functors[scalar_index][boundary] = inlet_functor;
  else
    mooseError("Unsupported inlet type on boundary " + boundary +
               (inlet_functor.empty() ? "" : ("\nInlet functor: " + inlet_functor)));
}

void
WCNSFVScalarTransportPhysics::addExternalScalarSources(
    std::vector<SubdomainName> blocks,
    std::vector<MooseFunctorName> scalar_sources,
    std::vector<MooseFunctorName> scalar_sources_coefs)
{
  _passive_scalar_coupled_sources_blocks.push_back(blocks);
  _passive_scalar_coupled_sources.push_back(scalar_sources);
  _passive_scalar_coupled_sources_coefs.push_back(scalar_sources_coefs);
}

void
WCNSFVScalarTransportPhysics::addScalarOutletBC()
{
  // Advection outlet is naturally handled by the advection flux kernel
  return;
}
