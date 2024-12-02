//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVFlowPhysicsBase.h"
#include "WCNSFVTurbulencePhysics.h"
#include "NSFVBase.h"
#include "MapConversionUtils.h"
#include "NS.h"

InputParameters
WCNSFVFlowPhysicsBase::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params.addClassDescription("Base class for Physics defining the Navier Stokes flow equations");

  params += NSFVBase::commonMomentumEquationParams();
  params.addParam<bool>("add_flow_equations",
                        true,
                        "Whether to add the flow equations. This parameter is not necessary when "
                        "using the Physics syntax");

  // We pull in parameters from various flow objects. This helps make sure the parameters are
  // spelled the same way and match the evolution of other objects.
  // If we remove these objects, or change their parameters, these parameters should be updated
  // Downstream actions must either implement all these options, or redefine the parameter with
  // a restricted MooseEnum, or place an error in the constructor for unsupported configurations
  // We mostly pull the boundary parameters from NSFV Action

  params += NSFVBase::commonNavierStokesFlowParams();
  params.addParam<bool>(
      "include_deviatoric_stress",
      false,
      "Whether to include the full expansion (the transposed term as well) of the stress tensor");

  // Momentum boundary conditions are important for advection problems as well
  params += NSFVBase::commonMomentumBoundaryTypesParams();

  // Specify the weakly compressible boundary flux information. They are used for specifying in flux
  // boundary conditions for advection physics in WCNSFV
  params += NSFVBase::commonMomentumBoundaryFluxesParams();
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "momentum_wall_functors",
      {},
      "Functors for each component of the velocity value on walls. This is only necessary for the "
      "fixed-velocity momentum wall types.");

  // Most downstream physics implementations are valid for porous media too
  // If yours is not, please remember to disable the 'porous_medium_treatment' parameter
  params.transferParam<bool>(NSFVBase::validParams(), "porous_medium_treatment");
  params.transferParam<MooseFunctorName>(NSFVBase::validParams(), "porosity");

  // New functor boundary conditions
  params.deprecateParam("momentum_inlet_function", "momentum_inlet_functors", "01/01/2025");
  params.deprecateParam("pressure_function", "pressure_functors", "01/01/2025");

  // Initialization parameters
  params.transferParam<std::vector<FunctionName>>(NSFVBase::validParams(), "initial_velocity");
  params.transferParam<FunctionName>(NSFVBase::validParams(), "initial_pressure");

  // Spatial discretization scheme
  // Specify the numerical schemes for interpolations of velocity and pressure
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "velocity_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "momentum_advection_interpolation");
  params.transferParam<bool>(NSFVBase::validParams(), "momentum_two_term_bc_expansion");
  params.transferParam<bool>(NSFVBase::validParams(), "pressure_two_term_bc_expansion");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("mu_interp_method",
                             coeff_interp_method,
                             "Switch that can select face interpolation method for the viscosity.");

  // Parameter groups
  params.addParamNamesToGroup(
      "velocity_variable pressure_variable initial_pressure initial_velocity", "Variables");
  params.addParamNamesToGroup("density dynamic_viscosity", "Material properties");
  params.addParamNamesToGroup("inlet_boundaries momentum_inlet_types momentum_inlet_functors",
                              "Inlet boundary conditions");
  params.addParamNamesToGroup("outlet_boundaries momentum_outlet_types pressure_functors",
                              "Outlet boundary conditions");
  params.addParamNamesToGroup("wall_boundaries momentum_wall_types momentum_wall_functors",
                              "Wall boundary conditions");
  params.addParamNamesToGroup(
      "velocity_interpolation momentum_advection_interpolation "
      "momentum_two_term_bc_expansion pressure_two_term_bc_expansion mu_interp_method",
      "Numerical scheme");
  params.addParamNamesToGroup("thermal_expansion", "Gravity treatment");

  return params;
}

WCNSFVFlowPhysicsBase::WCNSFVFlowPhysicsBase(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    _has_flow_equations(getParam<bool>("add_flow_equations")),
    _compressibility(getParam<MooseEnum>("compressibility")),
    _solve_for_dynamic_pressure(getParam<bool>("solve_for_dynamic_pressure")),
    _porous_medium_treatment(getParam<bool>("porous_medium_treatment")),
    _porosity_name(getParam<MooseFunctorName>("porosity")),
    _flow_porosity_functor_name(_porosity_name),
    _velocity_names(
        isParamValid("velocity_variable")
            ? getParam<std::vector<std::string>>("velocity_variable")
            : (_porous_medium_treatment
                   ? std::vector<std::string>(NS::superficial_velocity_vector,
                                              NS::superficial_velocity_vector + 3)
                   : std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3))),
    _pressure_name(isParamValid("pressure_variable")
                       ? getParam<NonlinearVariableName>("pressure_variable")
                       : NS::pressure),
    _fluid_temperature_name(isParamValid("fluid_temperature_variable")
                                ? getParam<NonlinearVariableName>("fluid_temperature_variable")
                                : NS::T_fluid),
    _density_name(getParam<MooseFunctorName>("density")),
    _density_gravity_name(isParamValid("density_gravity")
                              ? getParam<MooseFunctorName>("density_gravity")
                              : getParam<MooseFunctorName>("density")),
    _dynamic_viscosity_name(getParam<MooseFunctorName>("dynamic_viscosity")),
    _velocity_interpolation(getParam<MooseEnum>("velocity_interpolation")),
    _momentum_advection_interpolation(getParam<MooseEnum>("momentum_advection_interpolation")),
    _inlet_boundaries(getParam<std::vector<BoundaryName>>("inlet_boundaries")),
    _outlet_boundaries(getParam<std::vector<BoundaryName>>("outlet_boundaries")),
    _wall_boundaries(getParam<std::vector<BoundaryName>>("wall_boundaries")),
    _flux_inlet_pps(getParam<std::vector<PostprocessorName>>("flux_inlet_pps")),
    _flux_inlet_directions(getParam<std::vector<Point>>("flux_inlet_directions"))
{
  // Inlet boundary parameter checking
  checkSecondParamSetOnlyIfFirstOneSet("flux_inlet_pps", "flux_inlet_directions");
  if (_flux_inlet_directions.size())
    checkVectorParamsSameLengthIfSet<PostprocessorName, Point>("flux_inlet_pps",
                                                               "flux_inlet_directions");

  // Boussinesq parameters checks
  checkSecondParamSetOnlyIfFirstOneTrue("boussinesq_approximation", "ref_temperature");

  // Dynamic pressure parameter checks
  if (_compressibility != "incompressible" && _solve_for_dynamic_pressure)
    paramError("compressibility",
               "Solving for dynamic pressure is only implemented for incompressible flow");

  // Boundary parameters checking
  checkVectorParamAndMultiMooseEnumLength<BoundaryName>("inlet_boundaries", "momentum_inlet_types");
  checkVectorParamAndMultiMooseEnumLength<BoundaryName>("outlet_boundaries",
                                                        "momentum_outlet_types");
  checkVectorParamAndMultiMooseEnumLength<BoundaryName>("wall_boundaries", "momentum_wall_types");
  checkVectorParamLengthSameAsCombinedOthers<BoundaryName,
                                             std::vector<MooseFunctorName>,
                                             PostprocessorName>(
      "inlet_boundaries", "momentum_inlet_functors", "flux_inlet_pps");
  checkVectorParamsNoOverlap<BoundaryName>(
      {"inlet_boundaries", "outlet_boundaries", "wall_boundaries"});

  // Porous media parameters
  checkSecondParamSetOnlyIfFirstOneTrue("porous_medium_treatment", "porosity");

  if (_define_variables && _porous_medium_treatment)
    for (const auto & name : NS::velocity_vector)
    {
      const auto & it = std::find(_velocity_names.begin(), _velocity_names.end(), name);
      if (it != _velocity_names.end())
        paramError("velocity_variable",
                   "For porous medium simulations, functor name " + *it +
                       " is already reserved for the automatically-computed interstitial velocity. "
                       "Please choose another name for your external velocity variable!");
    }

  // Create maps for boundary-restricted parameters
  _momentum_inlet_types = Moose::createMapFromVectorAndMultiMooseEnum<BoundaryName>(
      _inlet_boundaries, getParam<MultiMooseEnum>("momentum_inlet_types"));
  _momentum_outlet_types = Moose::createMapFromVectorAndMultiMooseEnum<BoundaryName>(
      _outlet_boundaries, getParam<MultiMooseEnum>("momentum_outlet_types"));
  _momentum_wall_types = Moose::createMapFromVectorAndMultiMooseEnum<BoundaryName>(
      _wall_boundaries, getParam<MultiMooseEnum>("momentum_wall_types"));
  if (isParamSetByUser("momentum_inlet_functors"))
  {
    // Not all inlet boundary types require the specification of an inlet functor
    std::vector<BoundaryName> inlet_boundaries_with_functors;
    for (const auto & boundary : _inlet_boundaries)
      if (libmesh_map_find(_momentum_inlet_types, boundary) == "fixed-velocity" ||
          libmesh_map_find(_momentum_inlet_types, boundary) == "fixed-pressure")
        inlet_boundaries_with_functors.push_back(boundary);
    _momentum_inlet_functors =
        Moose::createMapFromVectors<BoundaryName, std::vector<MooseFunctorName>>(
            inlet_boundaries_with_functors,
            getParam<std::vector<std::vector<MooseFunctorName>>>("momentum_inlet_functors"));
  }
  if (isParamSetByUser("pressure_functors"))
  {
    // Not all outlet boundary types require the specification of an inlet functor
    std::vector<BoundaryName> outlet_boundaries_with_functors;
    for (const auto & boundary : _outlet_boundaries)
      if (libmesh_map_find(_momentum_outlet_types, boundary) == "fixed-pressure-zero-gradient" ||
          libmesh_map_find(_momentum_outlet_types, boundary) == "fixed-pressure")
        outlet_boundaries_with_functors.push_back(boundary);
    const auto & pressure_functors = getParam<std::vector<MooseFunctorName>>("pressure_functors");
    if (outlet_boundaries_with_functors.size() != pressure_functors.size())
      paramError("pressure_functors",
                 "Size (" + std::to_string(pressure_functors.size()) +
                     ") is not the same as the number of pressure outlet boundaries in "
                     "'fixed-pressure/fixed-pressure-zero-gradient' (size " +
                     std::to_string(outlet_boundaries_with_functors.size()) + ")");
    _pressure_functors = Moose::createMapFromVectors<BoundaryName, MooseFunctorName>(
        outlet_boundaries_with_functors, pressure_functors);
  }

  if (isParamSetByUser("momentum_wall_functors"))
  {
    // Not all wall boundary types require the specification of an inlet functor
    std::vector<BoundaryName> wall_boundaries_with_functors;
    for (const auto & boundary : _wall_boundaries)
      if (libmesh_map_find(_momentum_wall_types, boundary) == "noslip")
        wall_boundaries_with_functors.push_back(boundary);
    const auto & momentum_wall_functors =
        getParam<std::vector<std::vector<MooseFunctorName>>>("momentum_wall_functors");
    if (wall_boundaries_with_functors.size() != momentum_wall_functors.size())
      paramError("momentum_wall_functors",
                 "Size (" + std::to_string(momentum_wall_functors.size()) +
                     ") is not the same as the number of momentum_wall wall boundaries with "
                     "no-slip boundary conditions ' (size " +
                     std::to_string(wall_boundaries_with_functors.size()) + ")");

    _momentum_wall_functors =
        Moose::createMapFromVectors<BoundaryName, std::vector<MooseFunctorName>>(
            wall_boundaries_with_functors, momentum_wall_functors);
  }
}

void
WCNSFVFlowPhysicsBase::initializePhysicsAdditional()
{
  getProblem().needFV();
}

void
WCNSFVFlowPhysicsBase::actOnAdditionalTasks()
{
  // Turbulence physics would not be initialized before this task
  if (_current_task == "get_turbulence_physics")
    _turbulence_physics = getCoupledTurbulencePhysics();
}

void
WCNSFVFlowPhysicsBase::addFVBCs()
{
  addINSInletBC();
  addINSOutletBC();
  addINSWallsBC();
}

void
WCNSFVFlowPhysicsBase::addMaterials()
{
  if (hasForchheimerFriction() || _porous_medium_treatment)
    addPorousMediumSpeedMaterial();
  else
    addNonPorousMediumSpeedMaterial();
}

void
WCNSFVFlowPhysicsBase::addPorousMediumSpeedMaterial()
{
  InputParameters params = getFactory().getValidParams("PINSFVSpeedFunctorMaterial");
  assignBlocks(params, _blocks);

  for (unsigned int dim_i = 0; dim_i < dimension(); ++dim_i)
    params.set<MooseFunctorName>(NS::superficial_velocity_vector[dim_i]) = _velocity_names[dim_i];
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
  else
    params.set<MooseFunctorName>(NS::porosity) = "1";
  params.set<bool>("define_interstitial_velocity_components") = _porous_medium_treatment;

  getProblem().addMaterial("PINSFVSpeedFunctorMaterial", prefix() + "pins_speed_material", params);
}

void
WCNSFVFlowPhysicsBase::addNonPorousMediumSpeedMaterial()
{
  const std::string class_name = "ADVectorMagnitudeFunctorMaterial";
  InputParameters params = getFactory().getValidParams(class_name);
  assignBlocks(params, _blocks);

  const std::vector<std::string> param_names{"x_functor", "y_functor", "z_functor"};
  for (unsigned int dim_i = 0; dim_i < dimension(); ++dim_i)
    params.set<MooseFunctorName>(param_names[dim_i]) = _velocity_names[dim_i];
  params.set<MooseFunctorName>("vector_magnitude_name") = NS::speed;

  getProblem().addMaterial(class_name, prefix() + "ins_speed_material", params);
}

void
WCNSFVFlowPhysicsBase::addInitialConditions()
{
  if (!_define_variables && parameters().isParamSetByUser("initial_velocity") &&
      parameters().isParamSetByUser("velocity_variable") &&
      getParam<std::vector<FunctionName>>("initial_velocity").size() != 0)
    // TODO: Rework and remove this last statement once the NSFV action is removed
    paramError("initial_velocity",
               "Velocity is defined externally of WCNSFVFlowPhysicsBase, so should the inital "
               "conditions");
  if (!_define_variables && parameters().isParamSetByUser("initial_pressure") &&
      parameters().isParamSetByUser("pressure_variable"))
    paramError("initial_pressure",
               "Pressure is defined externally of WCNSFVFlowPhysicsBase, so should the inital "
               "condition");

  // Check dimension
  if (getParam<std::vector<FunctionName>>("initial_velocity").size() != dimension() &&
      getParam<std::vector<FunctionName>>("initial_velocity").size() != 3 &&
      getParam<std::vector<FunctionName>>("initial_velocity").size() != 0)
    // TODO: Rework and remove this last statement once the NSFV action is removed
    paramError("initial_velocity",
               "The number of velocity components in the " + type() + " initial condition is not " +
                   std::to_string(dimension()) + " or 3!");

  // do not set initial conditions if we load from file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;
  // do not set initial conditions if we are not defining variables
  if (!_define_variables)
    return;

  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);
  auto vvalue = getParam<std::vector<FunctionName>>("initial_velocity");

  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_velocity"))
    for (const auto d : make_range(dimension()))
    {
      params.set<VariableName>("variable") = _velocity_names[d];
      params.set<FunctionName>("function") = vvalue[d];

      getProblem().addInitialCondition("FunctionIC", prefix() + _velocity_names[d] + "_ic", params);
    }

  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_pressure"))
  {
    params.set<VariableName>("variable") = _pressure_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_pressure");

    getProblem().addInitialCondition("FunctionIC", prefix() + _pressure_name + "_ic", params);
  }
}

unsigned short
WCNSFVFlowPhysicsBase::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short ghost_layers = 2;
  return ghost_layers;
}

void
WCNSFVFlowPhysicsBase::addPostprocessors()
{
  const auto momentum_inlet_types = getParam<MultiMooseEnum>("momentum_inlet_types");

  for (unsigned int bc_ind = 0; bc_ind < momentum_inlet_types.size(); ++bc_ind)
    if (momentum_inlet_types[bc_ind] == "flux-mass" ||
        momentum_inlet_types[bc_ind] == "flux-velocity")
    {
      const std::string pp_type = "AreaPostprocessor";
      InputParameters params = getFactory().getValidParams(pp_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
      params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

      const auto name_pp = "area_pp_" + _inlet_boundaries[bc_ind];
      if (!getProblem().hasUserObject(name_pp))
        getProblem().addPostprocessor(pp_type, name_pp, params);
    }
}

VariableName
WCNSFVFlowPhysicsBase::getFlowVariableName(const std::string & short_name) const
{
  if (short_name == NS::pressure)
    return getPressureName();
  else if (short_name == NS::velocity_x && dimension() > 0)
    return getVelocityNames()[0];
  else if (short_name == NS::velocity_y && dimension() > 1)
    return getVelocityNames()[1];
  else if (short_name == NS::velocity_z && dimension() > 2)
    return getVelocityNames()[2];
  else if (short_name == NS::temperature)
    return getFluidTemperatureName();
  else
    mooseError("Short Variable name '", short_name, "' not recognized.");
}

MooseFunctorName
WCNSFVFlowPhysicsBase::getPorosityFunctorName(bool smoothed) const
{
  if (smoothed)
    return _flow_porosity_functor_name;
  else
    return _porosity_name;
}

const WCNSFVTurbulencePhysics *
WCNSFVFlowPhysicsBase::getCoupledTurbulencePhysics() const
{
  // User passed it, just use that
  if (isParamValid("coupled_turbulence_physics"))
    return getCoupledPhysics<WCNSFVTurbulencePhysics>(
        getParam<PhysicsName>("coupled_flow_physics"));
  // Look for any physics of the right type, and check the block restriction
  else
  {
    const auto all_turbulence_physics = getCoupledPhysics<const WCNSFVTurbulencePhysics>(true);
    for (const auto physics : all_turbulence_physics)
      if (checkBlockRestrictionIdentical(
              physics->name(), physics->blocks(), /*error_if_not_identical=*/false))
        return physics;
  }
  // Did not find one
  return nullptr;
}
