//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVScalarAdvectionPhysics.h"
#include "NSFVAction.h"
#include "WCNSFVFlowPhysics.h"

registerWCNSFVPhysicsBaseTasks("NavierStokesApp", WCNSFVScalarAdvectionPhysics);
registerMooseAction("NavierStokesApp", WCNSFVScalarAdvectionPhysics, "add_variable");
registerMooseAction("NavierStokesApp", WCNSFVScalarAdvectionPhysics, "add_ic");
registerMooseAction("NavierStokesApp", WCNSFVScalarAdvectionPhysics, "add_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVScalarAdvectionPhysics, "add_fv_bc");

InputParameters
WCNSFVScalarAdvectionPhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible scalar field advection equation(s)");

  params += NSFVAction::commonScalarFieldAdvectionParams();

  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "passive_scalar_inlet_functors",
      std::vector<std::vector<MooseFunctorName>>(),
      "Functors for inlet boundaries in the passive scalar equations.");

  // Functors can meet that need
  params.suppressParameter<std::vector<MooseFunctorName>>("passive_scalar_source");

  // Spatial finite volume discretization scheme
  params.transferParam<MooseEnum>(NSFVAction::validParams(),
                                  "passive_scalar_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "passive_scalar_face_interpolation");
  params.transferParam<bool>(NSFVAction::validParams(), "passive_scalar_two_term_bc_expansion");

  // Nonlinear equation solver scaling
  params.addRangeCheckedParam<std::vector<Real>>(
      "passive_scalar_scaling",
      "passive_scalar_scaling > 0.0",
      "The scaling factor for the passive scalar field variables.");

  return params;
}

WCNSFVScalarAdvectionPhysics::WCNSFVScalarAdvectionPhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters),
    _passive_scalar_names(getParam<std::vector<NonlinearVariableName>>("passive_scalar_names")),
    _passive_scalar_sources(
        getParam<std::vector<std::vector<MooseFunctorName>>>("passive_scalar_coupled_source")),
    _passive_scalar_sources_coef(
        getParam<std::vector<std::vector<Real>>>("passive_scalar_coupled_source_coeff")),
    _passive_scalar_inlet_types(getParam<MultiMooseEnum>("passive_scalar_inlet_types")),
    _passive_scalar_inlet_functors(
        getParam<std::vector<std::vector<MooseFunctorName>>>("passive_scalar_inlet_functors"))
{
  for (const auto & scalar_name : _passive_scalar_names)
    saveNonlinearVariableName(scalar_name);
  if (_flow_equations_physics)
    checkCommonParametersConsistent(_flow_equations_physics->parameters());

  // Dont let users pass empty vectors
  checkVectorParamNotEmpty<NonlinearVariableName>("passive_scalar_names");
  checkVectorParamNotEmpty<MooseFunctorName>("passive_scalar_diffusivity");

  // These parameters must be passed for every passive scalar at a time
  checkVectorParamsSameLength<NonlinearVariableName, MooseFunctorName>(
      "passive_scalar_names", "passive_scalar_diffusivity");
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, std::vector<MooseFunctorName>>(
      "passive_scalar_names", "passive_scalar_coupled_source");
  checkVectorParamsSameLengthIfSet<NonlinearVariableName, Real>("passive_scalar_names",
                                                                "passive_scalar_scaling");
  checkVectorParamsSameLength<NonlinearVariableName, std::vector<MooseFunctorName>>(
      "passive_scalar_names", "passive_scalar_inlet_functors");
  // checkTwoDVectorParamsSameLength<std::vector<std::string>,
  // std::string>("passive_scalar_inlet_functors",
  //                                                                "passive_scalar_inlet_types");
  checkTwoDVectorParamInnerSameLengthAsOneDVector<MooseFunctorName, BoundaryName>(
      "passive_scalar_inlet_functors", "inlet_boundaries");

  checkTwoDVectorParamsSameLength<MooseFunctorName, Real>("passive_scalar_coupled_source",
                                                          "passive_scalar_coupled_source_coeff");
}

void
WCNSFVScalarAdvectionPhysics::addNonlinearVariables()
{
  auto params = getFactory().getValidParams("INSFVScalarFieldVariable");
  assignBlocks(params, _blocks);
  params.set<MooseEnum>("face_interp_method") =
      getParam<MooseEnum>("passive_scalar_face_interpolation");
  params.set<bool>("two_term_boundary_expansion") =
      getParam<bool>("passive_scalar_two_term_bc_expansion");

  for (const auto name_i : index_range(_passive_scalar_names))
  {
    // Dont add if the user already defined the variable
    if (nonLinearVariableExists(_passive_scalar_names[name_i], /*error_if_aux=*/true))
      continue;

    if (isParamValid("passive_scalar_scaling"))
      params.set<std::vector<Real>>("scaling") = {
          getParam<std::vector<Real>>("passive_scalar_scaling")[name_i]};

    getProblem().addVariable("INSFVScalarFieldVariable", _passive_scalar_names[name_i], params);
  }
}

void
WCNSFVScalarAdvectionPhysics::addFVKernels()
{
  if (isTransient())
    addScalarTimeKernels();

  addScalarAdvectionKernels();
  addScalarDiffusionKernels();
  if (isParamValid("passive_scalar_coupled_source"))
    addScalarSourceKernels();
}

void
WCNSFVScalarAdvectionPhysics::addScalarTimeKernels()
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
WCNSFVScalarAdvectionPhysics::addScalarAdvectionKernels()
{
  const std::string kernel_type = "INSFVScalarFieldAdvection";
  InputParameters params = getFactory().getValidParams(kernel_type);

  assignBlocks(params, _blocks);
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("passive_scalar_advection_interpolation");

  for (const auto & vname : _passive_scalar_names)
  {
    params.set<NonlinearVariableName>("variable") = vname;
    getProblem().addFVKernel(kernel_type, prefix() + "ins_" + vname + "_advection", params);
  }
}

void
WCNSFVScalarAdvectionPhysics::addScalarDiffusionKernels()
{
  const std::string kernel_type = "FVDiffusion";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  for (const auto name_i : index_range(_passive_scalar_names))
  {
    params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
    params.set<MooseFunctorName>("coeff") =
        getParam<std::vector<MooseFunctorName>>("passive_scalar_diffusivity")[name_i];
    getProblem().addFVKernel(
        kernel_type, prefix() + "ins_" + _passive_scalar_names[name_i] + "_diffusion", params);
  }
}

void
WCNSFVScalarAdvectionPhysics::addScalarSourceKernels()
{
  const std::string kernel_type = "FVCoupledForce";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  for (const auto scalar_i : index_range(_passive_scalar_names))
  {
    for (const auto i : index_range(_passive_scalar_sources[scalar_i]))
    {

      params.set<NonlinearVariableName>("variable") = _passive_scalar_names[scalar_i];
      params.set<MooseFunctorName>("v") = _passive_scalar_sources[scalar_i][i];
      params.set<Real>("coef") = _passive_scalar_sources_coef[scalar_i][i];

      getProblem().addFVKernel(kernel_type,
                               prefix() + "ins_" + _passive_scalar_names[scalar_i] +
                                   "_coupled_source_" + std::to_string(i),
                               params);
    }
  }
}

void
WCNSFVScalarAdvectionPhysics::addFVBCs()
{
  addScalarInletBC();
  // There is typically no wall flux of passive scalars, similarly we rarely know
  // their concentrations at the outlet at the beginning of the simulation
  // TODO: we will know the outlet values in case of flow reversal. Implement scalar outlet
}

void
WCNSFVScalarAdvectionPhysics::addScalarInletBC()
{
  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    unsigned int flux_bc_counter = 0;
    unsigned int num_inlets = _inlet_boundaries.size();
    for (unsigned int bc_ind = 0; bc_ind < num_inlets; ++bc_ind)
    {
      if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "fixed-value")
      {
        const std::string bc_type = "FVFunctionDirichletBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<FunctionName>("function") = _passive_scalar_inlet_functors[name_i][bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        getProblem().addFVBC(
            bc_type, _passive_scalar_names[name_i] + "_" + _inlet_boundaries[bc_ind], params);
      }
      else if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass" ||
               _passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-velocity")
      {
        const auto flux_inlet_directions = getParam<std::vector<Point>>("flux_inlet_directions");
        const auto flux_inlet_pps = getParam<std::vector<PostprocessorName>>("flux_inlet_pps");

        const std::string bc_type = "WCNSFVScalarFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<MooseFunctorName>("passive_scalar") = _passive_scalar_names[name_i];
        if (flux_inlet_directions.size())
          params.set<Point>("direction") = flux_inlet_directions[flux_bc_counter];
        if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
        }
        else
          params.set<PostprocessorName>("velocity_pp") = flux_inlet_pps[flux_bc_counter];

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<PostprocessorName>("scalar_value_pp") =
            _passive_scalar_inlet_functors[name_i][bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        params.set<MooseFunctorName>(NS::velocity_x) = _velocity_names[0];
        if (dimension() > 1)
          params.set<MooseFunctorName>(NS::velocity_y) = _velocity_names[1];
        if (dimension() > 2)
          params.set<MooseFunctorName>(NS::velocity_z) = _velocity_names[2];

        getProblem().addFVBC(bc_type,
                             prefix() + _passive_scalar_names[name_i] + "_" +
                                 _inlet_boundaries[bc_ind],
                             params);
        flux_bc_counter += 1;
      }
    }
  }
}

void
WCNSFVScalarAdvectionPhysics::addInitialConditions()
{
  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);

  // We want to set ICs only if the user specified them
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
WCNSFVScalarAdvectionPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers =
      std::max(necessary_layers, WCNSFVPhysicsBase::getNumberAlgebraicGhostingLayersNeeded());
  if (getParam<MooseEnum>("passive_scalar_face_interpolation") == "skewness-corrected")
    necessary_layers = std::max(necessary_layers, (unsigned short)3);

  return necessary_layers;
}
