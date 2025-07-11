//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVFlowPhysics.h"
#include "WCNSFVTurbulencePhysics.h"
#include "NSFVBase.h"
#include "INSFVMomentumAdvection.h"
#include "RhieChowMassFlux.h"
#include "INSFVTimeKernel.h"
#include "MapConversionUtils.h"
#include "NS.h"

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp", WCNSLinearFVFlowPhysics);
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_linear_fv_bc");
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_functor_material");

InputParameters
WCNSLinearFVFlowPhysics::validParams()
{
  InputParameters params = WCNSFVFlowPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible equations with the linear "
      "solver implementation of the SIMPLE scheme");

  params.addParam<bool>(
      "orthogonality_correction", false, "Whether to use orthogonality correction");
  params.set<unsigned short>("ghost_layers") = 1;

  // This will be adapted based on the dimension
  params.set<std::vector<SolverSystemName>>("system_names") = {
      "u_system", "v_system", "w_system", "pressure_system"};

  // Implemented in the executioner
  params.suppressParameter<MooseEnum>("pinned_pressure_type");
  params.suppressParameter<Point>("pinned_pressure_point");
  params.suppressParameter<PostprocessorName>("pinned_pressure_value");

  // Not supported
  params.suppressParameter<bool>("add_flow_equations");
  params.set<bool>("porous_medium_treatment") = false;
  params.suppressParameter<bool>("porous_medium_treatment");
  params.set<MooseFunctorName>("porosity") = "1";
  params.suppressParameter<MooseFunctorName>("porosity");
  params.suppressParameter<MooseEnum>("mu_interp_method");
  // Not needed
  params.suppressParameter<bool>("add_flow_equations");
  params.suppressParameter<MooseEnum>("preconditioning");

  // No other options so far
  params.set<MooseEnum>("velocity_interpolation") = "rc";
  params.suppressParameter<MooseEnum>("velocity_interpolation");

  return params;
}

WCNSLinearFVFlowPhysics::WCNSLinearFVFlowPhysics(const InputParameters & parameters)
  : WCNSFVFlowPhysicsBase(parameters),
    _non_orthogonal_correction(getParam<bool>("orthogonality_correction"))
{
  if (_porous_medium_treatment)
    paramError("porous_medium_treatment", "Porous media unsupported");
  if (!_has_flow_equations)
    mooseError("Not supported");

  if (_hydraulic_separators.size())
    paramError("hydraulic_separator_sidesets",
               "Flow separators are not supported yet for linearFV!");
  if (getParam<bool>("pin_pressure"))
    paramError("pin_pressure",
               "Pressure pinning is implemented in the executioner for the linear finite volume "
               "segregated solves");
}

void
WCNSLinearFVFlowPhysics::initializePhysicsAdditional()
{
  WCNSFVFlowPhysicsBase::initializePhysicsAdditional();
  // TODO Add support for multi-system by either:
  // - creating the problem in the Physics or,
  // - checking that the right systems are being created
  getProblem().needSolutionState(2, Moose::SolutionIterationType::Nonlinear);
  // TODO Ban all other nonlinear Physics for now

  // Fix the default system names if using a different dimension
  if (!isParamSetByUser("system_name"))
  {
    if (dimension() == 1)
      _system_names = {"u_system", "pressure_system"};
    else if (dimension() == 2)
      _system_names = {"u_system", "v_system", "pressure_system"};
  }
}

void
WCNSLinearFVFlowPhysics::addSolverVariables()
{
  if (!_has_flow_equations)
    return;

  for (const auto d : make_range(dimension()))
    saveSolverVariableName(_velocity_names[d]);
  saveSolverVariableName(_pressure_name);

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
    if (!shouldCreateVariable(_velocity_names[d], _blocks, /*error if aux*/ true))
      reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
    else if (_define_variables)
    {
      std::string variable_type = "MooseLinearVariableFVReal";

      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks);
      params.set<SolverSystemName>("solver_sys") = getSolverSystem(_velocity_names[d]);

      getProblem().addVariable(variable_type, _velocity_names[d], params);
    }
    else
      paramError("velocity_variable",
                 "Variable (" + _velocity_names[d] +
                     ") supplied to the WCNSLinearFVFlowPhysics does not exist!");
  }

  // Pressure
  if (!shouldCreateVariable(_pressure_name, _blocks, /*error if aux*/ true))
    reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
  else if (_define_variables)
  {
    const auto pressure_type = "MooseLinearVariableFVReal";

    auto params = getFactory().getValidParams(pressure_type);
    assignBlocks(params, _blocks);
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(_pressure_name);

    getProblem().addVariable(pressure_type, _pressure_name, params);
  }
  else
    paramError("pressure_variable",
               "Variable (" + _pressure_name +
                   ") supplied to the WCNSLinearFVFlowPhysics does not exist!");
}

void
WCNSLinearFVFlowPhysics::addFVKernels()
{
  if (!_has_flow_equations)
    return;

  // Pressure correction equation: divergence of momentum
  addPressureCorrectionKernels();

  // Momentum equation: time derivative
  if (isTransient())
    addMomentumTimeKernels();

  // Momentum equation: flux terms
  addMomentumFluxKernels();

  // Momentum equation: pressure term
  addMomentumPressureKernels();

  // Momentum equation: friction term
  if (_friction_types.size())
    addMomentumFrictionKernels();

  // Momentum equation: gravity source term
  addMomentumGravityKernels();

  // Momentum equation: boussinesq approximation
  if (getParam<bool>("boussinesq_approximation"))
    addMomentumBoussinesqKernels();
}

void
WCNSLinearFVFlowPhysics::addPressureCorrectionKernels()
{
  {
    std::string kernel_type = "LinearFVAnisotropicDiffusion";
    std::string kernel_name = prefix() + "p_diffusion";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("diffusion_tensor") = "Ainv";
    params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }
  {
    std::string kernel_type = "LinearFVDivergence";
    std::string kernel_name = prefix() + "HbyA_divergence";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("face_flux") = "HbyA";
    params.set<bool>("force_boundary_execution") = true;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }
}

void
WCNSLinearFVFlowPhysics::addMomentumTimeKernels()
{
  std::string kernel_type = "LinearFVTimeDerivative";
  std::string kernel_name = prefix() + "ins_momentum_time";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("factor") = _density_name;

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    if (shouldCreateTimeDerivative(_velocity_names[d], _blocks, /*error if already defined*/ false))
      getProblem().addLinearFVKernel(kernel_type, kernel_name + "_" + NS::directions[d], params);
  }
}

void
WCNSLinearFVFlowPhysics::addMomentumFluxKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  std::string kernel_type = "LinearWCNSFVMomentumFlux";
  std::string kernel_name = prefix() + "ins_momentum_flux_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;
  params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;
  params.set<bool>("use_deviatoric_terms") = getParam<bool>("include_deviatoric_stress");

  for (unsigned int i = 0; i < dimension(); ++i)
    params.set<SolverVariableName>(u_names[i]) = _velocity_names[i];

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVFlowPhysics::addMomentumPressureKernels()
{
  std::string kernel_type = "LinearFVMomentumPressure";
  std::string kernel_name = prefix() + "ins_momentum_pressure_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<VariableName>("pressure") = _pressure_name;

  for (const auto d : make_range(dimension()))
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVFlowPhysics::addMomentumFrictionKernels()
{
  unsigned int num_friction_blocks = _friction_blocks.size();
  unsigned int num_used_blocks = num_friction_blocks ? num_friction_blocks : 1;

  const std::string kernel_type = "LinearFVMomentumFriction";
  InputParameters params = getFactory().getValidParams(kernel_type);

  for (const auto block_i : make_range(num_used_blocks))
  {
    std::string block_name = "";
    if (num_friction_blocks)
    {
      params.set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
      block_name = Moose::stringify(_friction_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    for (const auto d : make_range(dimension()))
    {
      params.set<LinearVariableName>("variable") = _velocity_names[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
      {
        const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
        if (upper_name == "DARCY")
        {
          params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
          params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
        }
        else
          paramError("momentum_friction_types",
                     "Friction type '",
                     _friction_types[block_i][type_i],
                     "' is not implemented");
      }

      getProblem().addLinearFVKernel(kernel_type,
                                     prefix() + "momentum_friction_" + block_name + "_" +
                                         NS::directions[d],
                                     params);
    }
  }
}

void
WCNSLinearFVFlowPhysics::addMomentumGravityKernels()
{
  if (parameters().isParamValid("gravity") && !_solve_for_dynamic_pressure)
  {
    std::string kernel_type = "LinearFVSource";
    std::string kernel_name = prefix() + "ins_momentum_gravity_";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    const auto gravity_vector = getParam<RealVectorValue>("gravity");
    const std::vector<std::string> comp_axis({"x", "y", "z"});

    for (const auto d : make_range(dimension()))
      if (gravity_vector(d) != 0)
      {
        params.set<MooseFunctorName>("source_density") = "rho_g_" + comp_axis[d];
        params.set<LinearVariableName>("variable") = _velocity_names[d];

        getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
      }
  }
}

void
WCNSLinearFVFlowPhysics::addMomentumBoussinesqKernels()
{
  if (_compressibility == "weakly-compressible")
    paramError("boussinesq_approximation",
               "We cannot use boussinesq approximation while running in weakly-compressible mode!");

  std::string kernel_type = "LinearFVMomentumBoussinesq";
  std::string kernel_name = prefix() + "ins_momentum_boussinesq_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<VariableName>(NS::T_fluid) = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _density_gravity_name;
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
  params.set<MooseFunctorName>("alpha_name") = getParam<MooseFunctorName>("thermal_expansion");

  for (const auto d : make_range(dimension()))
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<LinearVariableName>("variable") = _velocity_names[d];

    getProblem().addLinearFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSLinearFVFlowPhysics::addInletBC()
{
  // Check the size of the BC parameters
  unsigned int num_velocity_functor_inlets = 0;
  for (const auto & [bdy, momentum_inlet_type] : _momentum_inlet_types)
    if (momentum_inlet_type == "fixed-velocity" || momentum_inlet_type == "fixed-pressure")
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
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseFunctorName>("functor") = momentum_functors[d];

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + inlet_bdy, params);
      }
      ++velocity_pressure_counter;

      // Add the two term BC expansion for pressure if requested
      if (getParam<bool>("pressure_two_term_bc_expansion"))
      {
        const std::string bc_type = "LinearFVExtrapolatedPressureBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};
        params.set<LinearVariableName>("variable") = _pressure_name;
        params.set<bool>("use_two_term_expansion") = true;
        getProblem().addLinearFVBC(bc_type,
                                   _pressure_name + "_extrapolation_inlet_" +
                                       Moose::stringify(inlet_bdy),
                                   params);
      }
    }
    else if (momentum_inlet_type == "fixed-pressure")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _pressure_name;
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
WCNSLinearFVFlowPhysics::addOutletBC()
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
    // Zero tangeantial gradient condition on velocity
    if (momentum_outlet_type == "zero-gradient" || momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionOutflowBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
      params.set<bool>("use_two_term_expansion") = getParam<bool>("momentum_two_term_bc_expansion");

      for (const auto d : make_range(dimension()))
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
      }
    }

    // Fixed pressure condition, coming in the pressure correction equation
    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>("functor") = libmesh_map_find(_pressure_functors, outlet_bdy);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};

      getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
    }
  }
}

void
WCNSLinearFVFlowPhysics::addWallsBC()
{
  const std::string u_names[3] = {"u", "v", "w"};

  for (const auto & [boundary_name, momentum_wall_type] : _momentum_wall_types)
  {
    if (momentum_wall_type == "noslip")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};

      for (const auto d : make_range(dimension()))
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        if (_momentum_wall_functors.count(boundary_name) == 0)
          params.set<MooseFunctorName>("functor") = "0";
        else
          params.set<MooseFunctorName>("functor") = _momentum_wall_functors[boundary_name][d];

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
      }
    }
    else
      mooseError("Unsupported wall boundary condition type: " + std::string(momentum_wall_type));
  }

  if (getParam<bool>("pressure_two_term_bc_expansion"))
  {
    const std::string bc_type = "LinearFVExtrapolatedPressureBC";
    InputParameters params = getFactory().getValidParams(bc_type);
    params.set<std::vector<BoundaryName>>("boundary") = _wall_boundaries;
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<bool>("use_two_term_expansion") = true;
    getProblem().addLinearFVBC(
        bc_type, _pressure_name + "_extrapolation_" + Moose::stringify(_wall_boundaries), params);
  }
}

void
WCNSLinearFVFlowPhysics::addUserObjects()
{
  // Rhie Chow user object for interpolation velocities
  addRhieChowUserObjects();
}

void
WCNSLinearFVFlowPhysics::addRhieChowUserObjects()
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
  const auto object_type = "RhieChowMassFlux";

  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;
  params.set<std::string>("p_diffusion_kernel") = prefix() + "p_diffusion";
  params.set<MooseFunctorName>(NS::density) = _density_name;

  getProblem().addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSLinearFVFlowPhysics::addFunctorMaterials()
{
  if (parameters().isParamValid("gravity"))
  {
    const auto gravity_vector = getParam<RealVectorValue>("gravity");
    const std::vector<std::string> comp_axis({"x", "y", "z"});
    for (const auto d : make_range(dimension()))
      if (gravity_vector(d) != 0)
      {
        // Add rho * g functor for each relevant direction
        // TODO: we could avoid using an AD functor material for non-AD density functor
        auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
        assignBlocks(params, _blocks);
        params.set<std::string>("expression") =
            _density_gravity_name + " * " + std::to_string(gravity_vector(d));
        if (!MooseUtils::parsesToReal(_density_gravity_name))
          params.set<std::vector<std::string>>("functor_names") = {_density_gravity_name};
        params.set<std::string>("property_name") = "rho_g_" + comp_axis[d];
        // We don't output this helper material
        getProblem().addMaterial(
            "ADParsedFunctorMaterial", prefix() + "gravity_helper_" + comp_axis[d], params);
      }
  }
}

UserObjectName
WCNSLinearFVFlowPhysics::rhieChowUOName() const
{
  mooseAssert(!_porous_medium_treatment, "Not implemented");
  return "ins_rhie_chow_interpolator";
}

unsigned short
WCNSLinearFVFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return 1;
}
