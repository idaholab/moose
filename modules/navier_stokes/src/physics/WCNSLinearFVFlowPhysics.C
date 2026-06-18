//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "WCNSLinearFVFlowPhysics.h"

#include "MapConversionUtils.h"
#include "MooseUtils.h"
#include "NS.h"
#include "RhieChowMassFlux.h"
#include "TheWarehouse.h"

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp", WCNSLinearFVFlowPhysics);
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_linear_fv_bc");
registerMooseAction("NavierStokesApp", WCNSLinearFVFlowPhysics, "add_functor_material");

namespace
{
const std::string velocity_param_names[3] = {"u", "v", "w"};
}

InputParameters
WCNSLinearFVFlowPhysics::validParams()
{
  InputParameters params = WCNSFVFlowPhysicsBase::validParams();

  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible equations with the linear "
      "solver implementation of the SIMPLE scheme");

  params.addParam<bool>(
      "orthogonality_correction", false, "Whether to use orthogonality correction");
  params.renameParam("orthogonality_correction", "use_nonorthogonal_correction", "");
  params.set<unsigned short>("ghost_layers") = 1;

  params.set<std::vector<SolverSystemName>>("system_names") = {
      "u_system", "v_system", "w_system", "pressure_system"};

  params.suppressParameter<MooseEnum>("pinned_pressure_type");
  params.suppressParameter<Point>("pinned_pressure_point");
  params.suppressParameter<PostprocessorName>("pinned_pressure_value");

  params.suppressParameter<bool>("add_flow_equations");
  params.set<bool>("porous_medium_treatment") = false;
  params.suppressParameter<bool>("porous_medium_treatment");
  params.set<MooseFunctorName>("porosity") = "1";
  params.suppressParameter<MooseFunctorName>("porosity");
  params.suppressParameter<MooseEnum>("mu_interp_method");
  params.suppressParameter<MooseEnum>("preconditioning");

  params.set<MooseEnum>("velocity_interpolation") = "rc";
  params.suppressParameter<MooseEnum>("velocity_interpolation");

  params.transferParam<MooseEnum>(RhieChowMassFlux::validParams(), "pressure_projection_method");

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
               "Flow separators are not supported yet for linear FV.");
  if (getParam<bool>("pin_pressure"))
    paramError("pin_pressure",
               "Pressure pinning is implemented in the executioner for the linear finite volume "
               "segregated solves");
}

void
WCNSLinearFVFlowPhysics::initializePhysicsAdditional()
{
  WCNSFVFlowPhysicsBase::initializePhysicsAdditional();

  getProblem().needSolutionState(2, Moose::SolutionIterationType::Nonlinear);

  const std::vector<SolverSystemName> default_system_names = {
      "u_system", "v_system", "w_system", "pressure_system"};
  if (!isParamSetByUser("system_names") || _system_names == default_system_names)
  {
    if (dimension() == 1)
      _system_names = {"u_system", "pressure_system"};
    else if (dimension() == 2)
      _system_names = {"u_system", "v_system", "pressure_system"};
    else if (dimension() == 3)
      _system_names = {"u_system", "v_system", "w_system", "pressure_system"};
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

  if (_velocity_names.size() != dimension() && _velocity_names.size() != 3)
    paramError("velocity_variable",
               "The number of velocity variable names supplied to the NSFVAction is not " +
                   Moose::stringify(dimension()) + " (mesh dimension)" +
                   ((dimension() == 3) ? "" : " or 3!") + "\nVelocity variables " +
                   Moose::stringify(_velocity_names));

  for (const auto d : make_range(dimension()))
  {
    if (!shouldCreateVariable(_velocity_names[d], _blocks, /*error if aux*/ true))
      reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
    else if (_define_variables)
    {
      const std::string variable_type = "MooseLinearVariableFVReal";
      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks);
      params.set<SolverSystemName>("solver_sys") = getSolverSystem(_velocity_names[d]);

      getProblem().addVariable(variable_type, _velocity_names[d], params);
    }
    else
      paramError("velocity_variable",
                 "Variable (" + _velocity_names[d] +
                     ") supplied to the linear FV flow physics does not exist!");
  }

  if (!shouldCreateVariable(_pressure_name, _blocks, /*error if aux*/ true))
    reportPotentiallyMissedParameters({"system_names"}, "MooseLinearVariableFVReal");
  else if (_define_variables)
  {
    const std::string pressure_type = "MooseLinearVariableFVReal";
    auto params = getFactory().getValidParams(pressure_type);
    assignBlocks(params, _blocks);
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(_pressure_name);

    getProblem().addVariable(pressure_type, _pressure_name, params);
  }
  else
    paramError("pressure_variable",
               "Variable (" + _pressure_name +
                   ") supplied to the linear FV flow physics does not exist!");
}

void
WCNSLinearFVFlowPhysics::addFVKernels()
{
  if (!_has_flow_equations)
    return;

  addPressureCorrectionKernels();

  if (isTransient())
    addMomentumTimeKernels();

  addMomentumFluxKernels();
  if (isTransient() && useMomentumContinuityErrorSink())
    addMomentumConditioningKernels();

  if (shouldAddMomentumPressureKernels())
    addMomentumPressureKernels();

  if (_friction_types.size())
    addMomentumFrictionKernels();

  addMomentumGravityKernels();

  if (shouldAddMomentumReducedPressureKernels())
    addMomentumReducedPressureKernels();

  if (getParam<bool>("boussinesq_approximation"))
    addMomentumBoussinesqKernels();
}

void
WCNSLinearFVFlowPhysics::addPressureCorrectionKernels()
{
  {
    const std::string kernel_type = "LinearFVAnisotropicDiffusion";
    const std::string kernel_name = prefix() + "p_diffusion";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("diffusion_tensor") = pressureDiffusionTensorName();
    params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }

  {
    const std::string kernel_type = "LinearFVDivergence";
    const std::string kernel_name = prefix() + "HbyA_divergence";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("face_flux") = pressureDivergenceFluxName();
    params.set<bool>("face_flux_is_integrated") = pressureDivergenceFluxIsIntegrated();
    params.set<bool>("force_boundary_execution") = true;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }
}

void
WCNSLinearFVFlowPhysics::addMomentumTimeKernels()
{
  const std::string kernel_type = momentumTimeKernelType();
  const std::string kernel_name = prefix() + "ins_momentum_time";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(momentumTimeDensityParameterName()) = _density_name;
  setMomentumTimeKernelParams(params);

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];
    if (shouldCreateTimeDerivative(_velocity_names[d], _blocks, /*error if already defined*/ false))
      getProblem().addLinearFVKernel(kernel_type, kernel_name + "_" + NS::directions[d], params);
  }
}

void
WCNSLinearFVFlowPhysics::setMomentumTimeKernelParams(InputParameters & /* params */) const
{
}

void
WCNSLinearFVFlowPhysics::setVelocitySolverVariableParams(InputParameters & params) const
{
  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<SolverVariableName>(velocity_param_names[d]) = _velocity_names[d];
}

void
WCNSLinearFVFlowPhysics::setVelocityVariableParams(InputParameters & params) const
{
  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(velocity_param_names[d]) = _velocity_names[d];
}

bool
WCNSLinearFVFlowPhysics::rhieChowUserObjectAppliesToBlocks(const RhieChowMassFlux & rc_obj) const
{
  return rc_obj.blocks() == _blocks || rc_obj.blocks().empty() || _blocks.empty();
}

bool
WCNSLinearFVFlowPhysics::isCompatibleRhieChowUserObject(const UserObject & obj) const
{
  return dynamic_cast<const RhieChowMassFlux *>(&obj);
}

void
WCNSLinearFVFlowPhysics::checkIncompatibleRhieChowUserObject(
    const RhieChowMassFlux & /* rc_obj */) const
{
}

void
WCNSLinearFVFlowPhysics::setRhieChowUserObjectParams(InputParameters & /* params */) const
{
}

void
WCNSLinearFVFlowPhysics::addMomentumFluxKernels()
{
  const std::string kernel_type = momentumFluxKernelType();
  const std::string kernel_name = prefix() + "ins_momentum_flux_";

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  if (!_turbulence_physics)
    params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
  else
    params.set<MooseFunctorName>(NS::mu) = NS::mu_eff;

  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  if (const auto mass_flux_functor = momentumFluxMassFluxFunctorName(); !mass_flux_functor.empty())
  {
    params.set<MooseFunctorName>("mass_flux_functor") = mass_flux_functor;
    params.set<bool>("mass_flux_is_integrated") = momentumFluxMassFluxFunctorIsIntegrated();
  }

  params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;
  params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;
  params.set<bool>("use_deviatoric_terms") = includeSymmetrizedViscousStress();

  setVelocitySolverVariableParams(params);

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
  const std::string kernel_type = "LinearFVMomentumPressure";
  const std::string kernel_name = prefix() + "ins_momentum_pressure_";

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
          paramError("friction_types",
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
    const std::string kernel_type = "LinearFVSource";
    const std::string kernel_name = prefix() + "ins_momentum_gravity_";

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

  const std::string kernel_type = "LinearFVMomentumBoussinesq";
  const std::string kernel_name = prefix() + "ins_momentum_boussinesq_";

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

MooseFunctorName
WCNSLinearFVFlowPhysics::inletVelocityFunctorName(const BoundaryName & boundary,
                                                  const unsigned int component) const
{
  return libmesh_map_find(_momentum_inlet_functors, boundary)[component];
}

MooseFunctorName
WCNSLinearFVFlowPhysics::wallVelocityFunctorName(const BoundaryName & boundary,
                                                 const unsigned int component) const
{
  if (_momentum_wall_functors.count(boundary) == 0)
    return "0";

  return _momentum_wall_functors.at(boundary)[component];
}

std::string
WCNSLinearFVFlowPhysics::momentumOutletBCType(const BoundaryName & /* boundary */,
                                              const MooseEnum & /* momentum_outlet_type */) const
{
  return "LinearFVAdvectionDiffusionOutflowBC";
}

void
WCNSLinearFVFlowPhysics::setMomentumOutletBCParams(InputParameters & /* params */,
                                                   const BoundaryName & /* boundary */,
                                                   const MooseEnum & /* momentum_outlet_type */,
                                                   const unsigned int /* component */) const
{
}

std::string
WCNSLinearFVFlowPhysics::pressureOutletBCType(const BoundaryName & /* boundary */,
                                              const MooseEnum & /* momentum_outlet_type */) const
{
  return "LinearFVAdvectionDiffusionFunctorDirichletBC";
}

void
WCNSLinearFVFlowPhysics::setPressureOutletBCParams(
    InputParameters & /* params */,
    const BoundaryName & /* boundary */,
    const MooseEnum & /* momentum_outlet_type */) const
{
}

void
WCNSLinearFVFlowPhysics::addWallPressureBC(const BoundaryName & /* boundary */,
                                           const MooseEnum & /* momentum_wall_type */)
{
}

void
WCNSLinearFVFlowPhysics::addInletBC()
{
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
        params.set<MooseFunctorName>("functor") = inletVelocityFunctorName(inlet_bdy, d);
        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + inlet_bdy, params);
      }
      ++velocity_pressure_counter;

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

  for (const auto & [outlet_bdy, momentum_outlet_type] : _momentum_outlet_types)
  {
    if (momentum_outlet_type == "zero-gradient" || momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = momentumOutletBCType(outlet_bdy, momentum_outlet_type);
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
      params.set<bool>("use_two_term_expansion") = getParam<bool>("momentum_two_term_bc_expansion");

      for (const auto d : make_range(dimension()))
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        setMomentumOutletBCParams(params, outlet_bdy, momentum_outlet_type, d);
        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
      }
    }

    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = pressureOutletBCType(outlet_bdy, momentum_outlet_type);
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>("functor") = libmesh_map_find(_pressure_functors, outlet_bdy);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
      setPressureOutletBCParams(params, outlet_bdy, momentum_outlet_type);
      getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
    }
  }
}

void
WCNSLinearFVFlowPhysics::addWallsBC()
{
  bool has_symmetry_bc = false;

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
        params.set<MooseFunctorName>("functor") = wallVelocityFunctorName(boundary_name, d);
        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
      }

      addWallPressureBC(boundary_name, momentum_wall_type);
    }
    else if (momentum_wall_type == "symmetry")
    {
      has_symmetry_bc = true;
      {
        const std::string bc_type = "LinearFVVelocitySymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        setVelocitySolverVariableParams(params);

        for (const auto d : make_range(dimension()))
        {
          params.set<LinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
        }
      }
      {
        const std::string bc_type = "LinearFVPressureSymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        params.set<LinearVariableName>("variable") = _pressure_name;
        params.set<MooseFunctorName>("HbyA_flux") = wallPressureSymmetryFluxName();
        getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + boundary_name, params);
      }
    }
    else
      mooseError("Unsupported wall boundary condition type: " + std::string(momentum_wall_type));
  }

  if (shouldAddWallPressureTwoTermExpansion())
  {
    if (!has_symmetry_bc)
    {
      const std::string bc_type = "LinearFVExtrapolatedPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = _wall_boundaries;
      params.set<LinearVariableName>("variable") = _pressure_name;
      params.set<bool>("use_two_term_expansion") = true;
      getProblem().addLinearFVBC(
          bc_type, _pressure_name + "_extrapolation_" + Moose::stringify(_wall_boundaries), params);
    }
    else
      for (const auto & [boundary_name, momentum_wall_type] : _momentum_wall_types)
        if (momentum_wall_type != "symmetry")
        {
          const std::string bc_type = "LinearFVExtrapolatedPressureBC";
          InputParameters params = getFactory().getValidParams(bc_type);
          params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
          params.set<LinearVariableName>("variable") = _pressure_name;
          params.set<bool>("use_two_term_expansion") = true;
          getProblem().addLinearFVBC(
              bc_type, _pressure_name + "_extrapolation_" + boundary_name, params);
        }
  }
}

void
WCNSLinearFVFlowPhysics::addUserObjects()
{
  addAdditionalUserObjects();
  addRhieChowUserObjects();
}

void
WCNSLinearFVFlowPhysics::addRhieChowUserObjects()
{
  mooseAssert(dimension(), "0-dimension not supported");

  std::vector<UserObject *> objs;
  getProblem()
      .theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);
  bool have_compatible_rc_uo = false;
  for (const auto & obj : objs)
    if (dynamic_cast<RhieChowMassFlux *>(obj))
    {
      const auto rc_obj = dynamic_cast<RhieChowMassFlux *>(obj);
      if (!rhieChowUserObjectAppliesToBlocks(*rc_obj))
        continue;

      if (isCompatibleRhieChowUserObject(*obj))
        have_compatible_rc_uo = true;
      else
        checkIncompatibleRhieChowUserObject(*rc_obj);
    }

  if (have_compatible_rc_uo)
    return;

  const auto object_type = rhieChowUserObjectType();

  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  setVelocityVariableParams(params);

  params.set<VariableName>("pressure") = _pressure_name;
  params.set<std::string>("p_diffusion_kernel") = prefix() + "p_diffusion";
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("pressure_projection_method") =
      getParam<MooseEnum>("pressure_projection_method");
  setRhieChowUserObjectParams(params);

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
        auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
        assignBlocks(params, _blocks);
        params.set<std::string>("expression") =
            _density_gravity_name + " * " + std::to_string(gravity_vector(d));
        if (!MooseUtils::parsesToReal(_density_gravity_name))
          params.set<std::vector<std::string>>("functor_names") = {_density_gravity_name};
        params.set<std::string>("property_name") = "rho_g_" + comp_axis[d];
        getProblem().addMaterial(
            "ADParsedFunctorMaterial", prefix() + "gravity_helper_" + comp_axis[d], params);
      }
  }
}

unsigned short
WCNSLinearFVFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return 1;
}
