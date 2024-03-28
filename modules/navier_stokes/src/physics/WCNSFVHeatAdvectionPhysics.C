//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVHeatAdvectionPhysics.h"
#include "WCNSFVFlowPhysics.h"
#include "NSFVAction.h"

registerWCNSFVPhysicsBaseTasks("NavierStokesApp", WCNSFVHeatAdvectionPhysics);
registerMooseAction("NavierStokesApp", WCNSFVHeatAdvectionPhysics, "add_variable");
registerMooseAction("NavierStokesApp", WCNSFVHeatAdvectionPhysics, "add_ic");
registerMooseAction("NavierStokesApp", WCNSFVHeatAdvectionPhysics, "add_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVHeatAdvectionPhysics, "add_fv_bc");
registerMooseAction("NavierStokesApp", WCNSFVHeatAdvectionPhysics, "add_material");

InputParameters
WCNSFVHeatAdvectionPhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription("Define the Navier Stokes weakly-compressible energy equation");

  params += NSFVAction::commonFluidEnergyEquationParams();

  // TODO Remove the parameter once NavierStokesFV syntax has been removed
  params.addParam<bool>("add_energy_equation",
                        "Whether to add the energy equation. This parameter is not necessary if "
                        "using the Physics syntax");

  // These parameters are not shared because the NSFVPhysics use functors
  params.addParam<std::vector<MooseFunctorName>>(
      "energy_inlet_function",
      std::vector<MooseFunctorName>(),
      "Functors for Dirichlet/Neumann inlet boundaries in the energy equation.");
  params.addParam<std::vector<MooseFunctorName>>(
      "energy_wall_function",
      std::vector<MooseFunctorName>(),
      "Functors for Dirichlet/Neumann wall boundaries in the energy equation.");

  // New functor boundary conditions
  params.deprecateParam("energy_inlet_function", "energy_inlet_functors", "01/01/2025");
  params.deprecateParam("energy_wall_function", "energy_wall_functors", "01/01/2025");

  // Spatial finite volume discretization scheme
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "energy_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "energy_face_interpolation");
  params.transferParam<bool>(NSFVAction::validParams(), "energy_two_term_bc_expansion");

  // Nonlinear equation solver scaling
  params.transferParam<Real>(NSFVAction::validParams(), "energy_scaling");

  return params;
}

WCNSFVHeatAdvectionPhysics::WCNSFVHeatAdvectionPhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters),
    _has_energy_equation(
        isParamValid("add_energy_equation")
            ? getParam<bool>("add_energy_equation")
            : (usingNavierStokesFVSyntax() ? isParamSetByUser("energy_inlet_function") : true)),
    _fluid_temperature_name(_flow_equations_physics->getFluidTemperatureName()),
    _specific_heat_name(getParam<MooseFunctorName>("specific_heat")),
    _thermal_conductivity_blocks(
        parameters.isParamValid("thermal_conductivity_blocks")
            ? getParam<std::vector<std::vector<SubdomainName>>>("thermal_conductivity_blocks")
            : std::vector<std::vector<SubdomainName>>()),
    _thermal_conductivity_name(getParam<std::vector<MooseFunctorName>>("thermal_conductivity")),
    _ambient_convection_blocks(
        getParam<std::vector<std::vector<SubdomainName>>>("ambient_convection_blocks")),
    _ambient_convection_alpha(getParam<std::vector<MooseFunctorName>>("ambient_convection_alpha")),
    _ambient_temperature(getParam<std::vector<MooseFunctorName>>("ambient_temperature")),
    _energy_inlet_types(getParam<MultiMooseEnum>("energy_inlet_types")),
    _energy_inlet_functors(getParam<std::vector<MooseFunctorName>>("energy_inlet_functors")),
    _energy_wall_types(getParam<MultiMooseEnum>("energy_wall_types")),
    _energy_wall_functors(getParam<std::vector<MooseFunctorName>>("energy_wall_functors"))
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  saveNonlinearVariableName(_fluid_temperature_name);

  // Parameter checks
  checkVectorParamsSameLengthIfSet<MooseFunctorName, MooseFunctorName>("ambient_convection_alpha",
                                                                       "ambient_temperature");
  checkSecondParamSetOnlyIfFirstOneSet("external_heat_source", "external_heat_source_coeff");

  if (isParamValid("energy_wall_types"))
    checkVectorParamAndMultiMooseEnumLength<MooseFunctorName>("energy_wall_functors",
                                                              "energy_wall_types");
}

void
WCNSFVHeatAdvectionPhysics::addNonlinearVariables()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  // Dont add if the user already defined the variable
  if (nonlinearVariableExists(_fluid_temperature_name,
                              /*error_if_aux=*/true))
  {
    checkBlockRestrictionIdentical(_fluid_temperature_name,
                                   getProblem().getVariable(0, _fluid_temperature_name).blocks());
  }
  else if (_define_variables)
  {
    auto params = getFactory().getValidParams("INSFVEnergyVariable");
    assignBlocks(params, _blocks);
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("energy_scaling")};
    params.set<MooseEnum>("face_interp_method") = getParam<MooseEnum>("energy_face_interpolation");
    params.set<bool>("two_term_boundary_expansion") =
        getParam<bool>("energy_two_term_bc_expansion");
    getProblem().addVariable("INSFVEnergyVariable", _fluid_temperature_name, params);
  }
  else
    paramError("fluid_temperature_variable",
               "Variable (" + _fluid_temperature_name +
                   ") supplied to the NavierStokesFV action does not exist!");
}

void
WCNSFVHeatAdvectionPhysics::addFVKernels()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  if (isTransient())
  {
    if (_flow_equations_physics->compressibility() == "incompressible")
      addINSEnergyTimeKernels();
    else
      addWCNSEnergyTimeKernels();
  }

  addINSEnergyAdvectionKernels();
  addINSEnergyHeatConductionKernels();
  if (getParam<std::vector<MooseFunctorName>>("ambient_temperature").size())
    addINSEnergyAmbientConvection();
  if (isParamValid("external_heat_source"))
    addINSEnergyExternalHeatSource();
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyTimeKernels()
{
  std::string kernel_type = "INSFVEnergyTimeDerivative";
  std::string kernel_name = prefix() + "ins_energy_time";

  if (_flow_equations_physics->porousMediumTreatment())
  {
    kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = prefix() + "pins_energy_time";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
  params.set<MooseFunctorName>(NS::time_deriv(NS::specific_enthalpy)) =
      NS::time_deriv(NS::specific_enthalpy);

  if (_flow_equations_physics->porousMediumTreatment())
  {
    params.set<MooseFunctorName>(NS::porosity) =
        _flow_equations_physics->getPorosityFunctorName(false);
    if (getProblem().hasFunctor(NS::time_deriv(_flow_equations_physics->densityName()),
                                /*thread_id=*/0))
    {
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) =
          NS::time_deriv(_flow_equations_physics->densityName());
      params.set<MooseFunctorName>(NS::specific_enthalpy) = NS::specific_enthalpy;
    }
    params.set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVHeatAdvectionPhysics::addWCNSEnergyTimeKernels()
{
  std::string en_kernel_type = "WCNSFVEnergyTimeDerivative";
  std::string kernel_name = prefix() + "wcns_energy_time";

  if (_flow_equations_physics->porousMediumTreatment())
  {
    en_kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = prefix() + "pwcns_energy_time";
  }

  InputParameters params = getFactory().getValidParams(en_kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
  params.set<MooseFunctorName>(NS::specific_enthalpy) = NS::specific_enthalpy;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) =
      NS::time_deriv(_flow_equations_physics->densityName());

  if (_flow_equations_physics->porousMediumTreatment())
  {
    params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
    params.set<MooseFunctorName>(NS::porosity) =
        _flow_equations_physics->getPorosityFunctorName(false);
    params.set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(en_kernel_type, kernel_name, params);
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyAdvectionKernels()
{
  std::string kernel_type = "INSFVEnergyAdvection";
  std::string kernel_name = prefix() + "ins_energy_advection";
  if (_flow_equations_physics->porousMediumTreatment())
  {
    kernel_type = "PINSFVEnergyAdvection";
    kernel_name = prefix() + "pins_energy_advection";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseEnum>("velocity_interp_method") =
      _flow_equations_physics->getVelocityInterpolationMethod();
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("energy_advection_interpolation");

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyHeatConductionKernels()
{
  bool vector_conductivity = processThermalConductivity();
  unsigned int num_blocks = _thermal_conductivity_blocks.size();
  unsigned int num_used_blocks = num_blocks ? num_blocks : 1;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_blocks)
      block_name = Moose::stringify(_thermal_conductivity_blocks[block_i]);
    else
      block_name = std::to_string(block_i);

    if (_flow_equations_physics->porousMediumTreatment())
    {
      const std::string kernel_type =
          vector_conductivity ? "PINSFVEnergyAnisotropicDiffusion" : "PINSFVEnergyDiffusion";

      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      std::string conductivity_name = vector_conductivity ? NS::kappa : NS::k;
      params.set<MooseFunctorName>(conductivity_name) = _thermal_conductivity_name[block_i];
      params.set<MooseFunctorName>(NS::porosity) =
          _flow_equations_physics->getPorosityFunctorName(true);

      getProblem().addFVKernel(
          kernel_type, prefix() + "pins_energy_diffusion_" + block_name, params);
    }
    else
    {
      const std::string kernel_type = "FVDiffusion";
      InputParameters params = getFactory().getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      params.set<MooseFunctorName>("coeff") = _thermal_conductivity_name[block_i];

      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_energy_diffusion_" + block_name, params);
    }
  }
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyAmbientConvection()
{
  unsigned int num_convection_blocks = _ambient_convection_blocks.size();
  unsigned int num_used_blocks = num_convection_blocks ? num_convection_blocks : 1;

  const std::string kernel_type = "PINSFVEnergyAmbientConvection";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
  params.set<bool>("is_solid") = false;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_convection_blocks)
    {
      params.set<std::vector<SubdomainName>>("block") = _ambient_convection_blocks[block_i];
      block_name = Moose::stringify(_ambient_convection_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    params.set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[block_i];
    params.set<MooseFunctorName>(NS::T_solid) = _ambient_temperature[block_i];

    getProblem().addFVKernel(kernel_type, prefix() + "ambient_convection_" + block_name, params);
  }
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyExternalHeatSource()
{
  const std::string kernel_type = "FVCoupledForce";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("v") = getParam<MooseFunctorName>("external_heat_source");
  params.set<Real>("coef") = getParam<Real>("external_heat_source_coeff");

  getProblem().addFVKernel(kernel_type, prefix() + "external_heat_source", params);
}

void
WCNSFVHeatAdvectionPhysics::addWCNSEnergyMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "WCNSFVMixingLengthEnergyDiffusion";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
  params.set<MooseFunctorName>(NS::specific_enthalpy) = NS::specific_enthalpy;
  params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;
  params.set<Real>("schmidt_number") = getParam<Real>("turbulent_prandtl");
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;

  for (unsigned int dim_i = 0; dim_i < dimension(); ++dim_i)
    params.set<MooseFunctorName>(u_names[dim_i]) =
        _flow_equations_physics->getVelocityNames()[dim_i];

  if (_flow_equations_physics->porousMediumTreatment())
    getProblem().addFVKernel(kernel_type, prefix() + "pins_energy_mixing_length_diffusion", params);
  else
    getProblem().addFVKernel(kernel_type, prefix() + "ins_energy_mixing_length_diffusion", params);
}

void
WCNSFVHeatAdvectionPhysics::addFVBCs()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  addINSEnergyInletBC();
  addINSEnergyWallBC();
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyInletBC()
{
  const auto & inlet_boundaries = _flow_equations_physics->getInletBoundaries();
  if (inlet_boundaries.size() != _energy_inlet_types.size())
    paramError("energy_inlet_types", "");
  if (inlet_boundaries.size() != _energy_inlet_functors.size())
    paramError("energy_inlet_functors", "");

  unsigned int flux_bc_counter = 0;
  for (const auto bc_ind : index_range(_energy_inlet_types))
  {
    if (_energy_inlet_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "FVADFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_inlet_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctionNeumannBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<FunctionName>("function") = _energy_inlet_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "flux-mass" ||
             _energy_inlet_types[bc_ind] == "flux-velocity")
    {
      const std::string bc_type = "WCNSFVEnergyFluxBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      const auto flux_inlet_directions = _flow_equations_physics->getFluxInletDirections();
      const auto flux_inlet_pps = _flow_equations_physics->getFluxInletPPs();

      if (flux_inlet_directions.size())
        params.set<Point>("direction") = flux_inlet_directions[flux_bc_counter];
      if (_energy_inlet_types[bc_ind] == "flux-mass")
      {
        params.set<PostprocessorName>("mdot_pp") = flux_inlet_pps[flux_bc_counter];
        params.set<PostprocessorName>("area_pp") = "area_pp_" + inlet_boundaries[bc_ind];
      }
      else
        params.set<PostprocessorName>("velocity_pp") = flux_inlet_pps[flux_bc_counter];

      params.set<PostprocessorName>("temperature_pp") = _energy_inlet_functors[bc_ind];
      params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
      params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;

      for (const auto d : make_range(dimension()))
        params.set<MooseFunctorName>(NS::velocity_vector[d]) =
            _flow_equations_physics->getVelocityNames()[d];

      params.set<std::vector<BoundaryName>>("boundary") = {inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + inlet_boundaries[bc_ind], params);
      flux_bc_counter += 1;
    }
  }
}

void
WCNSFVHeatAdvectionPhysics::addINSEnergyWallBC()
{
  const auto & wall_boundaries = _flow_equations_physics->getWallBoundaries();
  if (wall_boundaries.size() != _energy_wall_types.size())
    paramError("energy_wall_types", "");
  if (wall_boundaries.size() != _energy_wall_functors.size())
    paramError("energy_wall_functors", "");

  for (unsigned int bc_ind = 0; bc_ind < _energy_wall_types.size(); ++bc_ind)
  {
    if (_energy_wall_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "FVFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_wall_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + wall_boundaries[bc_ind], params);
    }
    else if (_energy_wall_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctorNeumannBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<MooseFunctorName>("functor") = _energy_wall_functors[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {wall_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + wall_boundaries[bc_ind], params);
    }
  }
}

bool
WCNSFVHeatAdvectionPhysics::processThermalConductivity()
{
  checkBlockwiseConsistency<MooseFunctorName>("thermal_conductivity_blocks",
                                              {"thermal_conductivity"});
  bool have_scalar = false;
  bool have_vector = false;

  for (unsigned int i = 0; i < _thermal_conductivity_name.size(); ++i)
  {
    // First, check if the name is just a number (only in case of isotropic conduction)
    if (MooseUtils::parsesToReal(_thermal_conductivity_name[i]))
      have_scalar = true;
    // Now we determine what kind of functor we are dealing with
    else
    {
      if (getProblem().hasFunctorWithType<ADReal>(_thermal_conductivity_name[i],
                                                  /*thread_id=*/0))
        have_scalar = true;
      else
      {
        if (getProblem().hasFunctorWithType<ADRealVectorValue>(_thermal_conductivity_name[i],
                                                               /*thread_id=*/0))
          have_vector = true;
        else
        {
          paramError("thermal_conductivity",
                     "We only allow functor of type ADReal or ADRealVectorValue for thermal "
                     "conductivity!");
        }
      }
    }
  }

  if (have_vector && !_flow_equations_physics->porousMediumTreatment())
    paramError("thermal_conductivity", "Cannot use anisotropic diffusion with non-porous flows!");

  if (have_vector == have_scalar)
    paramError("thermal_conductivity",
               "The entries on thermal conductivity shall either be scalars of vectors, mixing "
               "them is not supported!");
  return have_vector;
}

void
WCNSFVHeatAdvectionPhysics::addInitialConditions()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;
  if (!_define_variables && parameters().isParamSetByUser("initial_temperature"))
    paramError("initial_temperature",
               "T_fluid is defined externally of NavierStokesFV, so should the inital condition");

  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);

  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_temperature"))
  {
    params.set<VariableName>("variable") = _fluid_temperature_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_temperature");

    getProblem().addInitialCondition("FunctionIC", _fluid_temperature_name + "_ic", params);
  }
}

void
WCNSFVHeatAdvectionPhysics::addMaterials()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  InputParameters params = getFactory().getValidParams("INSFVEnthalpyFunctorMaterial");
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.set<MooseFunctorName>("temperature") = _fluid_temperature_name;

  getProblem().addMaterial(
      "INSFVEnthalpyFunctorMaterial", prefix() + "ins_enthalpy_material", params);
}

unsigned short
WCNSFVHeatAdvectionPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers =
      std::max(necessary_layers, _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded());
  if (getParam<MooseEnum>("energy_face_interpolation") == "skewness-corrected")
    necessary_layers = std::max(necessary_layers, (unsigned short)3);

  return necessary_layers;
}
