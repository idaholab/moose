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

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp", WCNSFVLinearFlowPhysics);
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVLinearFlowPhysics, "add_linear_fv_bc");

InputParameters
WCNSFVLinearFlowPhysics::validParams()
{
  InputParameters params = WCNSFVFlowPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible mass and momentum equations with the linear "
      "solver implementation of the SIMPLE scheme");

  params.addParam<bool>(
      "orthogonality_correction", false, "Whether to use orthogonality correction");

  // Not supported
  params.set<bool>("porous_medium_treatment") = false;
  params.suppressParameter<bool>("porous_medium_treatment");
  params.set<MooseFunctorName>("porosity") = "1";
  params.suppressParameter<MooseFunctorName>("porosity");

  return params;
}

WCNSFVLinearFlowPhysics::WCNSFVLinearFlowPhysics(const InputParameters & parameters)
  : WCNSFVFlowPhysicsBase(parameters),
    _non_orthogonal_correction(getParam<bool>("orthogonality_correction"))
{
  if (_porous_medium_treatment)
    paramError("porous_medium_treatment", "Porous media unsupported");
  if (!_has_flow_equations)
    mooseError("Not supported");
}

void
WCNSFVLinearFlowPhysics::initializePhysicsAdditional()
{
  WCNSFVFlowPhysicsBase::initializePhysicsAdditional();
  // TODO Check that the Problem has the right systems
  // at least until we make the Physics creates the problem
  // TODO Uncomment once k-eps Physics is merged
  // getProblem().setSavePreviousNLSolution(true);
  // TODO Ban all other nonlinear Physics for now
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
    if (linearVariableExists(_velocity_names[d], true))
      checkBlockRestrictionIdentical(_velocity_names[d],
                                     getProblem().getVariable(0, _velocity_names[d]).blocks());
    else if (_define_variables)
    {
      std::string variable_type = "MooseLinearVariableFVReal";

      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks); // TODO: check wrt components

      params.set<SolverSystemName>("solver_sys") = v_short[d] + "_system";
      getProblem().addVariable(variable_type, _velocity_names[d], params);
    }
    else
      paramError("velocity_variable",
                 "Variable (" + _velocity_names[d] +
                     ") supplied to the WCNSFVLinearFlowPhysics does not exist!");
  }

  // Pressure
  if (linearVariableExists(_pressure_name, true))
    checkBlockRestrictionIdentical(_pressure_name,
                                   getProblem().getVariable(0, _pressure_name).blocks());
  else if (_define_variables)
  {
    const auto pressure_type = "MooseLinearVariableFVReal";

    auto params = getFactory().getValidParams(pressure_type);
    assignBlocks(params, _blocks);

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
    params.set<LinearVariableName>("variable") = _pressure_name;
    params.set<MooseFunctorName>("diffusion_tensor") = "Ainv";
    params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;

    getProblem().addLinearFVKernel(kernel_type, kernel_name, params);
  }
  {
    std::string kernel_type = "LinearFVDivergence";
    std::string kernel_name = prefix() + "p_divergence";

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _pressure_name;
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
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") = _momentum_face_interpolation;
  params.set<bool>("use_nonorthogonal_correction") = _non_orthogonal_correction;

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
WCNSFVLinearFlowPhysics::addINSMomentumPressureKernels()
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
WCNSFVLinearFlowPhysics::addINSMomentumGravityKernels()
{
  // if (parameters().isParamValid("gravity"))
  // {
  //   std::string kernel_type = "INSFVMomentumGravity";
  //   std::string kernel_name = prefix() + "ins_momentum_gravity_";

  //   if (_porous_medium_treatment)
  //   {
  //     kernel_type = "PINSFVMomentumGravity";
  //     kernel_name = prefix() + "pins_momentum_gravity_";
  //   }

  //   InputParameters params = getFactory().getValidParams(kernel_type);
  //   assignBlocks(params, _blocks);
  //   params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  //   params.set<MooseFunctorName>(NS::density) = _density_gravity_name;
  //   params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  //   if (_porous_medium_treatment)
  //     params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  //   for (const auto d : make_range(dimension()))
  //   {
  //     if (getParam<RealVectorValue>("gravity")(d) != 0)
  //     {
  //       params.set<MooseEnum>("momentum_component") = NS::directions[d];
  //       params.set<NonlinearVariableName>("variable") = _velocity_names[d];

  //       getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  //     }
  //   }
  // }
}

void
WCNSFVLinearFlowPhysics::addINSMomentumBoussinesqKernels()
{
  // if (_compressibility == "weakly-compressible")
  //   paramError("boussinesq_approximation",
  //              "We cannot use boussinesq approximation while running in weakly-compressible mode!");

  // if (parameters().isParamValid("gravity"))
  // {
  //   std::string kernel_type = "INSFVMomentumBoussinesq";
  //   std::string kernel_name = prefix() + "ins_momentum_boussinesq_";

  //   if (_porous_medium_treatment)
  //   {
  //     kernel_type = "PINSFVMomentumBoussinesq";
  //     kernel_name = prefix() + "pins_momentum_boussinesq_";
  //   }

  //   InputParameters params = getFactory().getValidParams(kernel_type);
  //   assignBlocks(params, _blocks);
  //   params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  //   params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
  //   params.set<MooseFunctorName>(NS::density) = _density_gravity_name;
  //   params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  //   params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
  //   params.set<MooseFunctorName>("alpha_name") = getParam<MooseFunctorName>("thermal_expansion");
  //   if (_porous_medium_treatment)
  //     params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
  //   // User declared the flow to be incompressible, we have to trust them
  //   params.set<bool>("_override_constant_check") = true;

  //   for (const auto d : make_range(dimension()))
  //   {
  //     params.set<MooseEnum>("momentum_component") = NS::directions[d];
  //     params.set<LinearVariableName>("variable") = _velocity_names[d];

  //     getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  //   }
  // }
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
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseFunctorName>("functor") = momentum_functors[d];

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + inlet_bdy, params);
      }
      ++velocity_pressure_counter;
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
          params.set<LinearVariableName>("variable") = _velocity_names[d];
          getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
        }
      }
    }

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
        params.set<LinearVariableName>("variable") = _velocity_names[d];
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

UserObjectName
WCNSFVLinearFlowPhysics::rhieChowUOName() const
{
  mooseAssert(!_porous_medium_treatment, "Not implemented");
  return "ins_rhie_chow_interpolator";
}
