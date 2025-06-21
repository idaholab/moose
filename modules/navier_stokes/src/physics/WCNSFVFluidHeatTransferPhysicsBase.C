//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVFluidHeatTransferPhysicsBase.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "WCNSFVFlowPhysics.h"
#include "PINSFVEnergyAnisotropicDiffusion.h"
#include "NSFVBase.h"

InputParameters
WCNSFVFluidHeatTransferPhysicsBase::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params += WCNSFVCoupledAdvectionPhysicsHelper::validParams();
  params.addClassDescription("Define the Navier Stokes weakly-compressible energy equation");

  params += NSFVBase::commonFluidEnergyEquationParams();
  params.transferParam<bool>(PINSFVEnergyAnisotropicDiffusion::validParams(),
                             "effective_conductivity");

  // TODO Remove the parameter once NavierStokesFV syntax has been removed
  params.addParam<bool>("add_energy_equation",
                        "Whether to add the energy equation. This parameter is not necessary if "
                        "using the Physics syntax");
  params.addParam<bool>("solve_for_enthalpy",
                        false,
                        "Whether to solve for the enthalpy or the temperature of the fluid");
  params.addParam<NonlinearVariableName>(
      "fluid_temperature_variable", NS::T_fluid, "Name of the fluid temperature variable");

  // Initial conditions
  params.addParam<FunctionName>(
      "initial_enthalpy",
      "Initial value of the enthalpy variable, only to be used when solving for enthalpy");

  // New functor boundary conditions
  params.deprecateParam("energy_inlet_function", "energy_inlet_functors", "01/01/2025");
  params.deprecateParam("energy_wall_function", "energy_wall_functors", "01/01/2025");

  // Spatial finite volume discretization scheme
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "energy_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "energy_face_interpolation");
  params.transferParam<bool>(NSFVBase::validParams(), "energy_two_term_bc_expansion");

  params.addParamNamesToGroup("specific_heat thermal_conductivity thermal_conductivity_blocks "
                              "use_external_enthalpy_material",
                              "Material properties");
  params.addParamNamesToGroup("energy_advection_interpolation energy_face_interpolation "
                              "energy_two_term_bc_expansion",
                              "Numerical scheme");
  params.addParamNamesToGroup("energy_inlet_types energy_inlet_functors",
                              "Inlet boundary conditions");
  params.addParamNamesToGroup("energy_wall_types energy_wall_functors", "Wall boundary conditions");

  return params;
}

WCNSFVFluidHeatTransferPhysicsBase::WCNSFVFluidHeatTransferPhysicsBase(
    const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(this),
    _has_energy_equation(
        isParamValid("add_energy_equation")
            ? getParam<bool>("add_energy_equation")
            : (usingNavierStokesFVSyntax() ? isParamSetByUser("energy_inlet_function") : true)),
    _solve_for_enthalpy(getParam<bool>("solve_for_enthalpy")),
    _fluid_enthalpy_name(getSpecificEnthalpyName()),
    _fluid_temperature_name(getParam<NonlinearVariableName>("fluid_temperature_variable")),
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

  if (_solve_for_enthalpy)
    saveSolverVariableName(_fluid_enthalpy_name);
  else
    saveSolverVariableName(_fluid_temperature_name);

  // set block restrictions if not set by user
  // This should probably be done for all the coupled physics, tbd
  if (!isParamSetByUser("block"))
    _blocks = _flow_equations_physics->blocks();

  // Parameter checks
  checkSecondParamSetOnlyIfFirstOneTrue("solve_for_enthalpy", "initial_enthalpy");
  checkVectorParamsSameLengthIfSet<MooseFunctorName, MooseFunctorName>("ambient_convection_alpha",
                                                                       "ambient_temperature");
  checkSecondParamSetOnlyIfFirstOneSet("external_heat_source", "external_heat_source_coeff");

  // Check boundary parameters if provided.
  // The boundaries are checked again when the boundary conditions are added as we want
  // to be able to more boundary conditions to a Physics dynamically
  if (isParamValid("energy_inlet_types"))
    checkVectorParamAndMultiMooseEnumLength<MooseFunctorName>("energy_inlet_functors",
                                                              "energy_inlet_types");
  if (isParamSetByUser("energy_wall_boundaries"))
    checkVectorParamsSameLengthIfSet<BoundaryName, MooseFunctorName>(
        "energy_wall_boundaries", "energy_wall_functors", false);
  if (isParamValid("energy_wall_types"))
    checkVectorParamAndMultiMooseEnumLength<MooseFunctorName>("energy_wall_functors",
                                                              "energy_wall_types");
}

void
WCNSFVFluidHeatTransferPhysicsBase::addFVKernels()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  if (shouldCreateTimeDerivative(
          _fluid_temperature_name, _blocks, /*error if already defined*/ false))
    addEnergyTimeKernels();

  addEnergyAdvectionKernels();
  addEnergyHeatConductionKernels();
  if (getParam<std::vector<MooseFunctorName>>("ambient_temperature").size())
    addEnergyAmbientConvection();
  if (isParamValid("external_heat_source"))
    addEnergyExternalHeatSource();
}

void
WCNSFVFluidHeatTransferPhysicsBase::addFVBCs()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  addEnergyInletBC();
  addEnergyWallBC();
  addEnergyOutletBC();
  addEnergySeparatorBC();
}

void
WCNSFVFluidHeatTransferPhysicsBase::actOnAdditionalTasks()
{
  // Turbulence physics would not be initialized before this task
  if (_current_task == "get_turbulence_physics")
    _turbulence_physics = getCoupledTurbulencePhysics();
}

bool
WCNSFVFluidHeatTransferPhysicsBase::processThermalConductivity()
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
                                                  /*thread_id=*/0) ||
          getProblem().hasFunctorWithType<Real>(_thermal_conductivity_name[i],
                                                /*thread_id=*/0))
        have_scalar = true;
      else
      {
        if (getProblem().hasFunctorWithType<ADRealVectorValue>(_thermal_conductivity_name[i],
                                                               /*thread_id=*/0))
          have_vector = true;
        else
          paramError("thermal_conductivity",
                     "We only allow functor of type Real/ADReal or ADRealVectorValue for thermal "
                     "conductivity! Functor '" +
                         _thermal_conductivity_name[i] + "' is not of the requested type.");
      }
    }
  }

  if (have_vector && !_porous_medium_treatment)
    paramError("thermal_conductivity", "Cannot use anisotropic diffusion with non-porous flows!");

  if (have_vector == have_scalar)
    paramError("thermal_conductivity",
               "The entries on thermal conductivity shall either be scalars of vectors, mixing "
               "them is not supported!");
  return have_vector;
}

void
WCNSFVFluidHeatTransferPhysicsBase::addInitialConditions()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;
  if (!_define_variables && parameters().isParamSetByUser("initial_temperature"))
    paramError(
        "initial_temperature",
        "T_fluid is defined externally of WCNSFVFluidHeatTransferPhysicsBase, so should the inital "
        "condition");
  // do not set initial conditions if we are not defining variables
  if (!_define_variables)
    return;

  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);

  if (shouldCreateIC(_fluid_temperature_name,
                     _blocks,
                     /*whether IC is a default*/ !isParamSetByUser("initial_temperature"),
                     /*error if already an IC*/ isParamSetByUser("initial_temperature")))
  {
    params.set<VariableName>("variable") = _fluid_temperature_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_temperature");

    getProblem().addInitialCondition("FunctionIC", _fluid_temperature_name + "_ic", params);
  }
  if (parameters().isParamValid("initial_enthalpy") &&
      shouldCreateIC(_fluid_enthalpy_name,
                     _blocks,
                     /*whether IC is a default*/ false,
                     /*error if already an IC*/ true))
  {
    params.set<VariableName>("variable") = _fluid_enthalpy_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_enthalpy");

    getProblem().addInitialCondition("FunctionIC", _fluid_enthalpy_name + "_ic", params);
  }
}

void
WCNSFVFluidHeatTransferPhysicsBase::addMaterials()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  // Note that this material choice would not work for Newton-INSFV + solve_for_enthalpy
  const auto object_type =
      _solve_for_enthalpy ? "LinearFVEnthalpyFunctorMaterial" : "INSFVEnthalpyFunctorMaterial";

  InputParameters params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);

  if (_solve_for_enthalpy)
  {
    params.set<MooseFunctorName>(NS::pressure) = _flow_equations_physics->getPressureName();
    params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
    params.set<MooseFunctorName>(NS::specific_enthalpy) = _fluid_enthalpy_name;
    if (isParamValid(NS::fluid))
      params.set<UserObjectName>(NS::fluid) = getParam<UserObjectName>(NS::fluid);
    else
    {
      if (!getProblem().hasFunctor("h_from_p_T_functor", 0) ||
          !getProblem().hasFunctor("T_from_p_h_functor", 0))
        paramError(NS::fluid,
                   "Either 'fp' must be specified or the 'h_from_p_T_functor' and "
                   "'T_from_p_h_functor' must be defined outside the Physics");
      // Note: we could define those in the Physics if cp is constant
      params.set<MooseFunctorName>("h_from_p_T_functor") = "h_from_p_T_functor";
      params.set<MooseFunctorName>("T_from_p_h_functor") = "T_from_p_h_functor";
    }
  }
  else
  {
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
    params.set<MooseFunctorName>("temperature") = _fluid_temperature_name;
  }

  getProblem().addMaterial(object_type, prefix() + "enthalpy_material", params);
}

unsigned short
WCNSFVFluidHeatTransferPhysicsBase::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers =
      std::max(necessary_layers, _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded());
  if (getParam<MooseEnum>("energy_face_interpolation") == "skewness-corrected")
    necessary_layers = std::max(necessary_layers, (unsigned short)3);

  return necessary_layers;
}
