//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVLinearFlowPhysics.h"
#include "WCNSFVTurbulencePhysics.h"
#include "NSFVBase.h"
#include "INSFVMomentumAdvection.h"
#include "RhieChowMassFlux.h"
#include "INSFVTimeKernel.h"
#include "MapConversionUtils.h"
#include "NS.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSFVLinearFlowPhysics);
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_variable");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_ic");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_linear_fv_bc");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_material");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_user_object");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_postprocessor");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "get_turbulence_physics");

InputParameters
WCNSFVLinearFlowPhysics::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible mass and momentum equations with the linear "
      "solver implementation of the SIMPLE scheme");

  // TODO: group parameters in a WCNSFVFlowBase class

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

  // Time derivative term correction
  params.transferParam<bool>(INSFVTimeKernel::validParams(), "contribute_to_rc");
  params.addParamNamesToGroup("contribute_to_rc", "Advanced");

  // Used for flow mixtures, where one phase is solid / not moving under the action of gravity
  params.addParam<MooseFunctorName>(
      "density_for_gravity_terms",
      "If specified, replaces the 'density' for the Boussinesq and gravity momentum kernels");

  // Most downstream physics implementations are valid for porous media too
  // If yours is not, please remember to disable the 'porous_medium_treatment' parameter
  params.transferParam<bool>(NSFVBase::validParams(), "porous_medium_treatment");
  params.transferParam<MooseFunctorName>(NSFVBase::validParams(), "porosity");

  // Momentum boundary conditions are important for advection problems as well
  params += NSFVBase::commonMomentumBoundaryTypesParams();

  // Specify the weakly compressible boundary flux information. They are used for specifying in flux
  // boundary conditions for advection physics in WCNSFV
  params += NSFVBase::commonMomentumBoundaryFluxesParams();

  // New functor boundary conditions
  params.deprecateParam("momentum_inlet_function", "momentum_inlet_functors", "01/01/2025");
  params.deprecateParam("pressure_function", "pressure_functors", "01/01/2025");

  // Initialization parameters
  params.transferParam<std::vector<FunctionName>>(NSFVBase::validParams(), "initial_velocity");
  params.transferParam<FunctionName>(NSFVBase::validParams(), "initial_pressure");

  // Couple to turbulence physics
  params.addParam<PhysicsName>("coupled_turbulence_physics",
                               "Turbulence Physics coupled with the flow");

  // Spatial discretization scheme
  // Specify the numerical schemes for interpolations of velocity and pressure
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "velocity_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "pressure_face_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "momentum_face_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "mass_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "momentum_advection_interpolation");
  params.transferParam<Real>(INSFVMomentumAdvection::validParams(), "characteristic_speed");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("mu_interp_method",
                             coeff_interp_method,
                             "Switch that can select face interpolation method for the viscosity.");

  // Parameter groups
  params.addParamNamesToGroup(
      "velocity_variable pressure_variable initial_pressure initial_velocity", "Variables");
  params.addParamNamesToGroup("density dynamic_viscosity characteristic_speed",
                              "Material properties");
  params.addParamNamesToGroup("inlet_boundaries momentum_inlet_types momentum_inlet_functors",
                              "Inlet boundary conditions");
  params.addParamNamesToGroup("outlet_boundaries momentum_outlet_types pressure_functors",
                              "Outlet boundary conditions");
  params.addParamNamesToGroup("wall_boundaries momentum_wall_types", "Wall boundary conditions");
  params.addParamNamesToGroup("coupled_turbulence_physics", "Coupled Physics");
  params.addParamNamesToGroup(
      "velocity_interpolation mass_advection_interpolation momentum_advection_interpolation ",
      "Numerical scheme");
  // TODO Add default preconditioning and move scaling parameters to a preconditioning group
  params.addParamNamesToGroup("thermal_expansion", "Gravity treatment");

  return params;
}

WCNSFVLinearFlowPhysics::WCNSFVLinearFlowPhysics(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    _has_flow_equations(getParam<bool>("add_flow_equations")),
    _compressibility(getParam<MooseEnum>("compressibility")),
    _porous_medium_treatment(getParam<bool>("porous_medium_treatment")),
    _porosity_name(getParam<MooseFunctorName>("porosity")),
    // Note: porosity smoothing not yet supported
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
    _momentum_face_interpolation(getParam<MooseEnum>("momentum_advection_interpolation")),
    _non_orthogonal_correction(getParam<bool>("orthogonality_correction")),
    _inlet_boundaries(getParam<std::vector<BoundaryName>>("inlet_boundaries")),
    _outlet_boundaries(getParam<std::vector<BoundaryName>>("outlet_boundaries")),
    _wall_boundaries(getParam<std::vector<BoundaryName>>("wall_boundaries")),
    _momentum_wall_types(getParam<MultiMooseEnum>("momentum_wall_types")),
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
  checkSecondParamSetOnlyIfFirstOneTrue("boussinesq_approximation", "thermal_expansion");

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
    // Not all inlet boundary types require the specification of an inlet functor
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
}

void
WCNSFVLinearFlowPhysics::initializePhysicsAdditional()
{
  getProblem().needFV();
  // TODO Check that the Problem has the right systems
  // at least until we make the Physics creates the problem
  // TODO Uncomment once k-eps Physics is merged
  // getProblem().setSavePreviousNLSolution(true);
}

void
WCNSFVLinearFlowPhysics::actOnAdditionalTasks()
{
  // Turbulence physics would not be initialized before this task
  if (_current_task == "get_turbulence_physics")
    _turbulence_physics = getCoupledTurbulencePhysics();
}

void
WCNSFVLinearFlowPhysics::addNonlinearVariables()
{
  if (!_has_flow_equations)
    return;

  // TODO Rename to system variable
  for (const auto d : make_range(dimension()))
    saveNonlinearVariableName(_velocity_names[d]);
  saveNonlinearVariableName(_pressure_name);

  const std::vector<std::string> v_short = {"u", "v", "w"};

  // Check number of variables
  if (_velocity_names.size() != dimension() && _velocity_names.size() != 3)
    paramError("velocity_variable",
               "The number of velocity variable names supplied to the NSFVAction is not " +
                   Moose::stringify(dimension()) + " (mesh dimension)" +
                   ((dimension() == 3) ? "" : " or 3!") + "\nVelocity variables " +
                   Moose::stringify(_velocity_names));

  // Velocities
  for (const auto d : make_range(dimension()))
  {
    // TODO switch to linear
    if (nonlinearVariableExists(_velocity_names[d], true))
      checkBlockRestrictionIdentical(_velocity_names[d],
                                     getProblem().getVariable(0, _velocity_names[d]).blocks());
    else if (_define_variables)
    {
      std::string variable_type = "MooseLinearVariableFVReal";

      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks); // TODO: check wrt components
      // TODO check if used
      params.set<std::vector<Real>>("scaling") = {getParam<Real>("momentum_scaling")};

      params.set<SolverSystemName>("solver_sys") = v_short[d] + "_system";
      getProblem().addVariable(variable_type, _velocity_names[d], params);
    }
    else
      paramError("velocity_variable",
                 "Variable (" + _velocity_names[d] +
                     ") supplied to the WCNSFVLinearFlowPhysics does not exist!");
  }

  // Pressure
  if (nonlinearVariableExists(_pressure_name, true))
    checkBlockRestrictionIdentical(_pressure_name,
                                   getProblem().getVariable(0, _pressure_name).blocks());
  else if (_define_variables)
  {
    const auto pressure_type = "MooseLinearVariableFVReal";

    auto params = getFactory().getValidParams(pressure_type);
    assignBlocks(params, _blocks);
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("mass_scaling")};

    params.set<SolverSystemName>("solver_sys") = "pressure_system";
    getProblem().addVariable(pressure_type, _pressure_name, params);
  }
  else
    paramError("pressure_variable",
               "Variable (" + _pressure_name +
                   ") supplied to the WCNSFVLinearFlowPhysics does not exist!");
}

void
WCNSFVLinearFlowPhysics::addFVKernels()
{
  if (!_has_flow_equations)
    return;

  // Pressure correction equation: divergence of momentum
  addINSPressureCorrectionKernels();

  // Momentum equation: time derivative
  if (isTransient())
    mooseError("Transient terms not implemented");

  // Momentum equation: flux terms
  addINSMomentumFluxKernels();

  // Momentum equation: pressure term
  addINSMomentumPressureKernels();

  // Momentum equation: gravity source term
  addINSMomentumGravityKernels();

  // Momentum equation: boussinesq approximation
  if (getParam<bool>("boussinesq_approximation"))
    addINSMomentumBoussinesqKernels();
}

void
WCNSFVLinearFlowPhysics::addINSPressureCorrectionKernels()
{
  {
    std::string kernel_type = "LinearFVAnisotropicDiffusion";
    std::string kernel_name = prefix() + "p_diffusion";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("diffusion_tensor") = "Ainv";
    params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }
  {
    std::string kernel_type = "LinearFVDivergence";
    std::string kernel_name = prefix() + "p_divergence";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("face_flux") = "HbyA";
    params.set<bool>("force_boundary_execution") = true;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }
}

void
WCNSFVLinearFlowPhysics::addINSMomentumFluxKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  std::string kernel_type = "LinearWCNSFVMomentumFlux";
  std::string kernel_name = prefix() + "ins_momentum_flux_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") = _momentum_face_interpolation;
  params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;

  for (unsigned int i = 0; i < dimension(); ++i)
    params.set<MooseFunctorName>(u_names[i]) = _velocity_names[i];

  for (const auto d : make_range(dimension()))
  {
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSFVLinearFlowPhysics::addINSMomentumPressureKernels()
{
  std::string kernel_type = "LinearFVMomentumPressure";
  std::string kernel_name = prefix() + "ins_momentum_pressure_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("pressure") = _pressure_name;

  for (const auto d : make_range(dimension()))
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSFVLinearFlowPhysics::addINSMomentumGravityKernels()
{
  if (parameters().isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumGravity";
    std::string kernel_name = prefix() + "ins_momentum_gravity_";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumGravity";
      kernel_name = prefix() + "pins_momentum_gravity_";
    }

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
    params.set<MooseFunctorName>(NS::density) = _density_gravity_name;
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
    if (_porous_medium_treatment)
      params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (const auto d : make_range(dimension()))
    {
      if (getParam<RealVectorValue>("gravity")(d) != 0)
      {
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];

        getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
      }
    }
  }
}

void
WCNSFVLinearFlowPhysics::addINSMomentumBoussinesqKernels()
{
  if (_compressibility == "weakly-compressible")
    paramError("boussinesq_approximation",
               "We cannot use boussinesq approximation while running in weakly-compressible mode!");

  if (parameters().isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumBoussinesq";
    std::string kernel_name = prefix() + "ins_momentum_boussinesq_";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumBoussinesq";
      kernel_name = prefix() + "pins_momentum_boussinesq_";
    }

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
    params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
    params.set<MooseFunctorName>(NS::density) = _density_gravity_name;
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
    params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
    params.set<MooseFunctorName>("alpha_name") = getParam<MooseFunctorName>("thermal_expansion");
    if (_porous_medium_treatment)
      params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
    // User declared the flow to be incompressible, we have to trust them
    params.set<bool>("_override_constant_check") = true;

    for (const auto d : make_range(dimension()))
    {
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      params.set<NonlinearVariableName>("variable") = _velocity_names[d];

      getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
  }
}

void
WCNSFVLinearFlowPhysics::addFVBCs()
{
  addINSInletBC();
  addINSOutletBC();
  addINSWallsBC();
}

void
WCNSFVLinearFlowPhysics::addINSInletBC()
{
  // Check the size of the BC parameters
  unsigned int num_velocity_functor_inlets = 0;
  for (const auto & [bdy, momentum_outlet_type] : _momentum_inlet_types)
    if (momentum_outlet_type == "fixed-velocity" || momentum_outlet_type == "fixed-pressure")
      num_velocity_functor_inlets++;

  if (num_velocity_functor_inlets != _momentum_inlet_functors.size())
    paramError("momentum_inlet_functors",
               "Size (" + std::to_string(_momentum_inlet_functors.size()) +
                   ") is not the same as the number of entries in the momentum_inlet_types "
                   "subvector for fixed-velocities/pressures functors (size " +
                   std::to_string(num_velocity_functor_inlets) + ")");

  unsigned int velocity_pressure_counter = 0;
  for (const auto & [inlet_bdy, momentum_inlet_type] : _momentum_inlet_types)
  {
    if (momentum_inlet_type == "fixed-velocity")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};
      if (_momentum_inlet_functors.size() < velocity_pressure_counter + 1)
        paramError("momentum_inlet_functors",
                   "More non-flux inlets than inlet functors (" +
                       std::to_string(_momentum_inlet_functors.size()) + ")");

      // Check that enough functors have been provided for the dimension of the problem
      const auto momentum_functors = libmesh_map_find(_momentum_inlet_functors, inlet_bdy);
      if (momentum_functors.size() < dimension())
        paramError("momentum_inlet_functors",
                   "Subvector for boundary '" + inlet_bdy + "' (size " +
                       std::to_string(momentum_functors.size()) +
                       ") is not the same size as the number of dimensions of the physics (" +
                       std::to_string(dimension()) + ")");

      for (const auto d : make_range(dimension()))
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseFunctorName>("functor") = momentum_functors[d];

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + inlet_bdy, params);
      }
      ++velocity_pressure_counter;
    }
    else if (momentum_inlet_type == "fixed-pressure")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      if (_momentum_inlet_functors.size() < velocity_pressure_counter + 1)
        paramError("momentum_inlet_functors",
                   "More non-flux inlets than inlet functors (" +
                       std::to_string(_momentum_inlet_functors.size()) + ")");

      params.set<MooseFunctorName>("functor") =
          libmesh_map_find(_momentum_inlet_functors, inlet_bdy)[0];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};

      getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + inlet_bdy, params);
      ++velocity_pressure_counter;
    }
    else
      mooseError("Unsupported inlet boundary condition type: ", momentum_inlet_type);
  }
}

void
WCNSFVLinearFlowPhysics::addINSOutletBC()
{
  // Check the BCs size
  unsigned int num_pressure_outlets = 0;
  for (const auto & [bdy, momentum_outlet_type] : _momentum_outlet_types)
    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
      num_pressure_outlets++;

  if (num_pressure_outlets != _pressure_functors.size())
    paramError("pressure_functors",
               "Size (" + std::to_string(_pressure_functors.size()) +
                   ") is not the same as the number of pressure outlet boundaries in "
                   "'fixed-pressure/fixed-pressure-zero-gradient' (size " +
                   std::to_string(num_pressure_outlets) + ")");

  const std::string u_names[3] = {"u", "v", "w"};
  for (const auto & [outlet_bdy, momentum_outlet_type] : _momentum_outlet_types)
  {
    if (momentum_outlet_type == "zero-gradient" || momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      {
        const std::string bc_type = "LinearFVAdvectionDiffusionOutflowBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};

        for (const auto d : make_range(dimension()))
        {
          params.set<NonlinearVariableName>("variable") = _velocity_names[d];
          getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
        }
      }
    }

    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>("functor") = libmesh_map_find(_pressure_functors, outlet_bdy);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};

      getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
    }
  }
}

void
WCNSFVLinearFlowPhysics::addINSWallsBC()
{
  const std::string u_names[3] = {"u", "v", "w"};

  for (unsigned int bc_ind = 0; bc_ind < _momentum_wall_types.size(); ++bc_ind)
  {
    if (_momentum_wall_types[bc_ind] == "noslip")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (const auto d : make_range(dimension()))
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseFunctorName>("functor") = "0";

        getProblem().addLinearFVBC(
            bc_type, _velocity_names[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "wallfunction")
    {
      // Placeholder
      mooseError("Unsupported boundary condition type: " + _momentum_wall_types[bc_ind]);
    }
    else if (_momentum_wall_types[bc_ind] == "slip")
    {
      // Do nothing
    }
    else if (_momentum_wall_types[bc_ind] == "symmetry")
    {
      // Placeholder
      mooseError("Unsupported boundary condition type: " + _momentum_wall_types[bc_ind]);
    }
  }
}

void
WCNSFVLinearFlowPhysics::addUserObjects()
{
  // Rhie Chow user object for interpolation velocities
  addRhieChowUserObjects();

  if (!_has_flow_equations)
    return;

  // Pressure pin
  if (getParam<bool>("pin_pressure"))
  {
    const auto pin_type = getParam<MooseEnum>("pinned_pressure_type");
    std::string object_type = "NSPressurePin";

    // No need for the user object
    if (pin_type == "point-value" || pin_type == "average")
      return;

    // Create the average value postprocessor if needed
    if (pin_type == "average-uo")
    {
      // Volume average by default, but we could do inlet or outlet for example
      InputParameters params = getFactory().getValidParams("ElementAverageValue");
      params.set<std::vector<VariableName>>("variable") = {_pressure_name};
      assignBlocks(params, _blocks);
      params.set<std::vector<OutputName>>("outputs") = {"none"};
      getProblem().addPostprocessor("ElementAverageValue", "ns_pressure_average", params);
    }

    InputParameters params = getFactory().getValidParams(object_type);
    if (pin_type == "point-value" || pin_type == "point-value-uo")
      params.set<MooseEnum>("pin_type") = "point-value";
    else
      params.set<MooseEnum>("pin_type") = "average";

    params.set<PostprocessorName>("phi0") = getParam<PostprocessorName>("pinned_pressure_value");
    params.set<NonlinearVariableName>("variable") = _pressure_name;
    if (pin_type == "point-value" || pin_type == "point-value-uo")
      params.set<Point>("point") = getParam<Point>("pinned_pressure_point");
    else if (pin_type == "average-uo")
      params.set<PostprocessorName>("pressure_average") = "ns_pressure_average";

    getProblem().addUserObject(object_type, prefix() + "ins_mass_pressure_pin", params);
  }
}

void
WCNSFVLinearFlowPhysics::addMaterials()
{
  if (hasForchheimerFriction() || _porous_medium_treatment)
    addPorousMediumSpeedMaterial();
  else
    addNonPorousMediumSpeedMaterial();
}

void
WCNSFVLinearFlowPhysics::addPorousMediumSpeedMaterial()
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
WCNSFVLinearFlowPhysics::addNonPorousMediumSpeedMaterial()
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
WCNSFVLinearFlowPhysics::addInitialConditions()
{
  if (!_define_variables && parameters().isParamSetByUser("initial_velocity") &&
      parameters().isParamSetByUser("velocity_variable") &&
      getParam<std::vector<FunctionName>>("initial_velocity").size() != 0)
    // TODO: Rework and remove this last statement once the NSFV action is removed
    paramError("initial_velocity",
               "Velocity is defined externally of WCNSFVLinearFlowPhysics, so should the inital "
               "conditions");
  if (!_define_variables && parameters().isParamSetByUser("initial_pressure") &&
      parameters().isParamSetByUser("pressure_variable"))
    paramError("initial_pressure",
               "Pressure is defined externally of WCNSFVLinearFlowPhysics, so should the inital "
               "condition");

  // Check dimension
  if (getParam<std::vector<FunctionName>>("initial_velocity").size() != dimension() &&
      getParam<std::vector<FunctionName>>("initial_velocity").size() != 3 &&
      getParam<std::vector<FunctionName>>("initial_velocity").size() != 0)
    // TODO: Rework and remove this last statement once the NSFV action is removed
    paramError("initial_velocity",
               "The number of velocity components in the WCNSFVLinearFlowPhysics initial condition "
               "is not " +
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
WCNSFVLinearFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short ghost_layers = 2;
  if ((_porous_medium_treatment &&
       getParam<MooseEnum>("porosity_interface_pressure_treatment") != "automatic") ||
      getParam<MooseEnum>("momentum_face_interpolation") == "skewness-corrected" ||
      getParam<MooseEnum>("pressure_face_interpolation") == "skewness-corrected")
    ghost_layers = std::max(ghost_layers, (unsigned short)3);
  return ghost_layers;
}

void
WCNSFVLinearFlowPhysics::addRhieChowUserObjects()
{
  mooseAssert(dimension(), "0-dimension not supported");

  // First make sure that we only add this object once
  // Potential cases:
  // - there is a flow physics, and an advection one (UO should be added by one)
  // - there is only an advection physics (UO should be created)
  // - there are two advection physics on different blocks with set velocities (first one picks)
  // Counting RC UOs defined on the same blocks seems to be the most fool proof option
  std::vector<UserObject *> objs;
  getProblem()
      .theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);
  unsigned int num_rc_uo = 0;
  for (const auto & obj : objs)
    if (dynamic_cast<RhieChowMassFlux *>(obj))
    {
      const auto rc_obj = dynamic_cast<RhieChowMassFlux *>(obj);
      if (rc_obj->blocks() == _blocks)
        num_rc_uo++;
      // one of the RC user object is defined everywhere
      else if (rc_obj->blocks().size() == 0 || _blocks.size() == 0)
        num_rc_uo++;
    }

  if (num_rc_uo)
    return;

  const std::string u_names[3] = {"u", "v", "w"};
  const auto object_type =
      _porous_medium_treatment ? "PINSFVRhieChowMassFlux" : "INSFVRhieChowMassFlux";

  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;
  params.set<std::string>("p_diffusion_kernel") = prefix() + "p_diffusion";

  getProblem().addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSFVLinearFlowPhysics::checkRhieChowFunctorsDefined() const
{
  if (!getProblem().hasFunctor("ax", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ax must be provided for advection by auxiliary velocities");
  if (dimension() >= 2 && !getProblem().hasFunctor("ay", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ay must be provided for advection by auxiliary velocities");
  if (dimension() == 3 && !getProblem().hasFunctor("az", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient az must be provided for advection by auxiliary velocities");
}

void
WCNSFVLinearFlowPhysics::addPostprocessors()
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

UserObjectName
WCNSFVLinearFlowPhysics::rhieChowUOName() const
{
  return (_porous_medium_treatment ? +"pins_rhie_chow_interpolator" : "ins_rhie_chow_interpolator");
}

VariableName
WCNSFVLinearFlowPhysics::getFlowVariableName(const std::string & short_name) const
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
WCNSFVLinearFlowPhysics::getPorosityFunctorName(bool smoothed) const
{
  if (smoothed)
    return _flow_porosity_functor_name;
  else
    return _porosity_name;
}

const WCNSFVTurbulencePhysics *
WCNSFVLinearFlowPhysics::getCoupledTurbulencePhysics() const
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
