//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.addParam<NonlinearVariableName>(
      "fluid_temperature_variable", NS::T_fluid, "Name of the fluid temperature variable");

  // New functor boundary conditions
  params.deprecateParam("energy_inlet_function", "energy_inlet_functors", "01/01/2025");
  params.deprecateParam("energy_wall_function", "energy_wall_functors", "01/01/2025");

  // Spatial finite volume discretization scheme
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "energy_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "energy_face_interpolation");
  params.transferParam<bool>(NSFVBase::validParams(), "energy_two_term_bc_expansion");

  // Nonlinear equation solver scaling
  params.transferParam<Real>(NSFVBase::validParams(), "energy_scaling");

  params.addParamNamesToGroup("specific_heat thermal_conductivity thermal_conductivity_blocks "
                              "use_external_enthalpy_material",
                              "Material properties");
  params.addParamNamesToGroup("energy_advection_interpolation energy_face_interpolation "
                              "energy_two_term_bc_expansion energy_scaling",
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

  saveSolverVariableName(_fluid_temperature_name);

  // set block restrictions if not set by user
  // This should probably be done for all the coupled physics, tbd
  if (!isParamSetByUser("block"))
    _blocks = _flow_equations_physics->blocks();

  // Parameter checks
  checkVectorParamsSameLengthIfSet<MooseFunctorName, MooseFunctorName>("ambient_convection_alpha",
                                                                       "ambient_temperature");
  checkSecondParamSetOnlyIfFirstOneSet("external_heat_source", "external_heat_source_coeff");

  // Check boundary parameters if provided.
  // The boundaries are checked again when the boundary conditions are added as we want
  // to be able to more boundary conditions to a Physics dynamically
  if (isParamValid("energy_inlet_types"))
    checkVectorParamAndMultiMooseEnumLength<MooseFunctorName>("energy_inlet_functors",
                                                              "energy_inlet_types");
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

  if (isTransient())
  {
    if (_compressibility == "incompressible")
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
WCNSFVFluidHeatTransferPhysicsBase::addFVBCs()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  addINSEnergyInletBC();
  addINSEnergyWallBC();
  addINSEnergyOutletBC();
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
                                                  /*thread_id=*/0))
        have_scalar = true;
      else
      {
        if (getProblem().hasFunctorWithType<ADRealVectorValue>(_thermal_conductivity_name[i],
                                                               /*thread_id=*/0))
          have_vector = true;
        else
          paramError("thermal_conductivity",
                     "We only allow functor of type ADReal or ADRealVectorValue for thermal "
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
  // do not set initial conditions if we load from file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;
  // do not set initial conditions if we are not defining variables
  if (!_define_variables)
    return;

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
WCNSFVFluidHeatTransferPhysicsBase::addMaterials()
{
  // For compatibility with Modules/NavierStokesFV syntax
  if (!_has_energy_equation)
    return;

  InputParameters params = getFactory().getValidParams("INSFVEnthalpyFunctorMaterial");
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.set<MooseFunctorName>("temperature") = _fluid_temperature_name;

  getProblem().addMaterial(
      "INSFVEnthalpyFunctorMaterial", prefix() + "ins_enthalpy_material", params);
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
