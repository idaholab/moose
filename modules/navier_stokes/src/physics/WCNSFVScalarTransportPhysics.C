//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVScalarTransportPhysics.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "WCNSFVFlowPhysics.h"
#include "NSFVBase.h"
#include "NS.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSFVScalarTransportPhysics);
registerWCNSFVScalarTransportBaseTasks("NavierStokesApp", WCNSFVScalarTransportPhysics);

InputParameters
WCNSFVScalarTransportPhysics::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params += WCNSFVCoupledAdvectionPhysicsHelper::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible scalar field transport equation(s)");

  params += NSFVBase::commonScalarFieldAdvectionParams();

  // TODO Remove the parameter once NavierStokesFV syntax has been removed
  params.addParam<bool>(
      "add_scalar_equation",
      "Whether to add the scalar transport equation. This parameter is not necessary if "
      "using the Physics syntax");

  // These parameters are not shared because the NSFVPhysics use functors
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "passive_scalar_inlet_function",
      std::vector<std::vector<MooseFunctorName>>(),
      "Functors for inlet boundaries in the passive scalar equations.");

  // New functor boundary conditions
  params.deprecateParam(
      "passive_scalar_inlet_function", "passive_scalar_inlet_functors", "01/01/2025");

  // No need for the duplication
  params.addParam<std::vector<MooseFunctorName>>("passive_scalar_source", "Passive scalar sources");

  // Spatial finite volume discretization scheme
  params.transferParam<MooseEnum>(NSFVBase::validParams(),
                                  "passive_scalar_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "passive_scalar_face_interpolation");
  params.transferParam<bool>(NSFVBase::validParams(), "passive_scalar_two_term_bc_expansion");

  // Nonlinear equation solver scaling
  params.addRangeCheckedParam<std::vector<Real>>(
      "passive_scalar_scaling",
      "passive_scalar_scaling > 0.0",
      "The scaling factor for the passive scalar field variables.");

  // Parameter groups
  params.addParamNamesToGroup("passive_scalar_names initial_scalar_variables", "Variable");
  params.addParamNamesToGroup(
      "passive_scalar_advection_interpolation passive_scalar_face_interpolation "
      "passive_scalar_two_term_bc_expansion passive_scalar_scaling",
      "Numerical scheme");
  params.addParamNamesToGroup("passive_scalar_inlet_types passive_scalar_inlet_functors",
                              "Inlet boundary");

  return params;
}

WCNSFVScalarTransportPhysics::WCNSFVScalarTransportPhysics(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(this),
    _passive_scalar_names(getParam<std::vector<NonlinearVariableName>>("passive_scalar_names")),
    _has_scalar_equation(isParamValid("add_scalar_equation") ? getParam<bool>("add_scalar_equation")
                                                             : !usingNavierStokesFVSyntax()),
    _passive_scalar_inlet_types(getParam<MultiMooseEnum>("passive_scalar_inlet_types")),
    _passive_scalar_inlet_functors(
        getParam<std::vector<std::vector<MooseFunctorName>>>("passive_scalar_inlet_functors")),
    _passive_scalar_sources(getParam<std::vector<MooseFunctorName>>("passive_scalar_source")),
    _passive_scalar_coupled_sources(
        getParam<std::vector<std::vector<MooseFunctorName>>>("passive_scalar_coupled_source")),
    _passive_scalar_sources_coef(
        getParam<std::vector<std::vector<Real>>>("passive_scalar_coupled_source_coeff"))
{
  for (const auto & scalar_name : _passive_scalar_names)
    saveSolverVariableName(scalar_name);

  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  // These parameters must be passed for every passive scalar at a time
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, MooseFunctorName>(
      "passive_scalar_names", "passive_scalar_diffusivity", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, std::vector<MooseFunctorName>>(
      "passive_scalar_names", "passive_scalar_coupled_source", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, MooseFunctorName>(
      "passive_scalar_names", "passive_scalar_source", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, Real>(
      "passive_scalar_names", "passive_scalar_scaling", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, FunctionName>(
      "passive_scalar_names", "initial_scalar_variables", true);
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, std::vector<MooseFunctorName>>(
      "passive_scalar_names", "passive_scalar_inlet_functors", true);
  if (_passive_scalar_inlet_functors.size())
    checkTwoDVectorParamMultiMooseEnumSameLength<MooseFunctorName>(
        "passive_scalar_inlet_functors", "passive_scalar_inlet_types", false);

  if (_passive_scalar_sources_coef.size())
    checkTwoDVectorParamsSameLength<MooseFunctorName, Real>("passive_scalar_coupled_source",
                                                            "passive_scalar_coupled_source_coeff");

  if (_porous_medium_treatment)
    _flow_equations_physics->paramError("porous_medium_treatment",
                                        "Porous media scalar advection is currently unimplemented");
}

void
WCNSFVScalarTransportPhysics::addNonlinearVariables()
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

    if (isParamValid("passive_scalar_scaling"))
      params.set<std::vector<Real>>("scaling") = {
          getParam<std::vector<Real>>("passive_scalar_scaling")[name_i]};

    getProblem().addVariable("INSFVScalarFieldVariable", _passive_scalar_names[name_i], params);
  }
}

void
WCNSFVScalarTransportPhysics::addFVKernels()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  if (isTransient())
    addScalarTimeKernels();

  addScalarAdvectionKernels();
  addScalarDiffusionKernels();
  if (_passive_scalar_sources.size() || _passive_scalar_coupled_sources.size())
    addScalarSourceKernels();
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
WCNSFVScalarTransportPhysics::addFVBCs()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;

  addScalarInletBC();
  // There is typically no wall flux of passive scalars, similarly we rarely know
  // their concentrations at the outlet at the beginning of the simulation
  // TODO: we will know the outlet values in case of flow reversal. Implement scalar outlet
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
WCNSFVScalarTransportPhysics::addInitialConditions()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_scalar_equation)
    return;
  if (!_define_variables && parameters().isParamSetByUser("initial_scalar_variables"))
    paramError("initial_scalar_variables",
               "Scalar variables are defined externally of NavierStokesFV, so should their inital "
               "conditions");
  // do not set initial conditions if we load from file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;
  // do not set initial conditions if we are not defining variables
  if (!_define_variables)
    return;

  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);

  // There are no default initial conditions for passive scalar variables, we however
  // must obey the user-defined initial conditions, even if we are restarting
  if (parameters().isParamSetByUser("initial_scalar_variables"))
  {
    for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
    {
      params.set<VariableName>("variable") = _passive_scalar_names[name_i];
      params.set<FunctionName>("function") =
          getParam<std::vector<FunctionName>>("initial_scalar_variables")[name_i];

      getProblem().addInitialCondition("FunctionIC", _passive_scalar_names[name_i] + "_ic", params);
    }
  }
}

unsigned short
WCNSFVScalarTransportPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers =
      std::max(necessary_layers, _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded());
  if (getParam<MooseEnum>("passive_scalar_face_interpolation") == "skewness-corrected")
    necessary_layers = std::max(necessary_layers, (unsigned short)3);

  return necessary_layers;
}
