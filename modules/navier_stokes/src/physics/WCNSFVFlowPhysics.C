//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVFlowPhysics.h"
#include "NSFVAction.h"

InputParameters
WCNSFVFlowPhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible mass and momentum equations");

  // Pressure pin parameters
  params.transferParam<bool>(NSFVAction::validParams(), "pin_pressure");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "pinned_pressure_type");
  params.transferParam<Point>(NSFVAction::validParams(), "pinned_pressure_point");
  params.transferParam<PostprocessorName>(NSFVAction::validParams(), "pinned_pressure_value");

  // Initialization parameters
  params.transferParam<std::vector<FunctionName>>(NSFVAction::validParams(), "initial_velocity");
  params.transferParam<FunctionName>(NSFVAction::validParams(), "initial_pressure");

  // Techniques to limit or remove oscillations at porosity jump interfaces
  params.transferParam<MooseEnum>(NSFVAction::validParams(),
                                  "porosity_interface_pressure_treatment");

  // Friction correction, a technique to limit oscillations at friction interfaces
  params.transferParam<bool>(NSFVAction::validParams(), "use_friction_correction");
  params.transferParam<Real>(NSFVAction::validParams(), "consistent_scaling");

  // Boussinesq approximation
  params.transferParam<bool>(NSFVAction::validParams(), "boussinesq_approximation");
  params.transferParam<Real>(NSFVAction::validParams(), "ref_temperature");
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "thermal_expansion");

  // Volumetric friction terms, mostly used for porous media modeling
  params.transferParam<std::vector<std::vector<SubdomainName>>>(NSFVAction::validParams(),
                                                                "friction_blocks");
  params.transferParam<std::vector<std::vector<std::string>>>(NSFVAction::validParams(),
                                                              "friction_types");
  params.transferParam<std::vector<std::vector<std::string>>>(NSFVAction::validParams(),
                                                              "friction_coeffs");

  // Spatial discretization scheme
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "mass_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "momentum_advection_interpolation");

  // Nonlinear solver parameters
  params.transferParam<Real>(NSFVAction::validParams(), "mass_scaling");
  params.transferParam<Real>(NSFVAction::validParams(), "momentum_scaling");

  return params;
}

WCNSFVFlowPhysics::WCNSFVFlowPhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters)
{
  checkTwoDVectorParamsSameLength<BoundaryName, MooseEnum>("friction_blocks", "friction_types");
  checkTwoDVectorParamsSameLength<BoundaryName, MooseEnum>("friction_blocks", "friction_coeffs");

  checkParamsBothSetOrNotSet("pin_pressure", "pin_pressure_type");
  checkParamsBothSetOrNotSet("pin_pressure", "pin_pressure_value");
  // checkDependentParameterErrorContains("pin_pressure_type", "point", "pin_pressure_point");

  checkDependentParameterError("use_friction_correction", {"consistent_scaling"}, true);
  checkDependentParameterError(
      "boussinesq_approximation", {"ref_temperature", "thermal_expansion"}, true);
}

void
WCNSFVFlowPhysics::addNonlinearVariables()
{
  // Process parameters necessary to handle block-restriction
  processMesh();

  // Velocities
  for (const auto d : make_range(_dim))
  {
    if (nonLinearVariableExists(_velocity_names[d], true))
      continue;

    std::string variable_type = "INSFVVelocityVariable";
    if (_porous_medium_treatment)
      variable_type = "PINSFVSuperficialVelocityVariable";

    auto params = getFactory().getValidParams(variable_type);
    assignBlocks(params, blocks()); // TODO: check wrt components
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("momentum_scaling")};
    params.set<MooseEnum>("face_interp_method") =
        getParam<MooseEnum>("momentum_advection_interpolation");
    params.set<bool>("two_term_boundary_expansion") =
        getParam<bool>("momentum_two_term_bc_expansion");

    for (unsigned int d = 0; d < _dim; ++d)
      getProblem().addVariable(variable_type, _velocity_names[d], params);
  }

  // Pressure
  if (!nonLinearVariableExists(_pressure_name, true))
  {
    const bool using_pinsfv_pressure_var =
        _porous_medium_treatment &&
        getParam<MooseEnum>("porosity_interface_pressure_treatment") != "automatic";
    const auto pressure_type =
        using_pinsfv_pressure_var ? "BernoulliPressureVariable" : "INSFVPressureVariable";

    auto params = getFactory().getValidParams(pressure_type);
    assignBlocks(params, blocks()); // TODO: check wrt components
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("mass_scaling")};
    params.set<MooseEnum>("face_interp_method") =
        getParam<MooseEnum>("pressure_face_interpolation");
    params.set<bool>("two_term_boundary_expansion") =
        getParam<bool>("pressure_two_term_bc_expansion");

    if (using_pinsfv_pressure_var)
    {
      params.set<MooseFunctorName>("u") = _velocity_names[0];
      if (_dim >= 2)
        params.set<MooseFunctorName>("v") = _velocity_names[1];
      if (_dim == 3)
        params.set<MooseFunctorName>("w") = _velocity_names[2];
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<bool>("allow_two_term_expansion_on_bernoulli_faces") =
          getParam<bool>("allow_two_term_expansion_on_bernoulli_faces");
    }

    getProblem().addVariable(pressure_type, _pressure_name, params);
  }
}

void
WCNSFVFlowPhysics::addFVKernels()
{
  // Mass equation: time derivative
  if (_compressibility == "weakly-compressible")
    addWCNSMassTimeKernels();

  // Mass equation: divergence of momentum
  addINSMassKernels();

  // Momentum equation: time derivative
  if (_compressibility == "incompressible")
    addINSMomentumTimeKernels();
  else
    addWCNSMomentumTimeKernels();

  // Momentum equation: momentum advection
  addINSMomentumAdvectionKernels();

  // Momentum equation: momentum viscous stress
  addINSMomentumViscousDissipationKernels();

  // Momentum equation: pressure term
  addINSMomentumPressureKernels();

  // Momentum equation: gravity source term
  addINSMomentumGravityKernels();

  // Momentum equation: friction kernels
  if (getParam<std::vector<std::vector<SubdomainName>>>("friction_blocks").size())
    addINSMomentumFrictionKernels();

  // Momentum equation: boussinesq approximation
  if (getParam<bool>("boussinesq_approximation"))
    addINSMomentumBoussinesqKernels();
}

void
WCNSFVFlowPhysics::addWCNSMassTimeKernels()
{
  std::string mass_kernel_type = "WCNSFVMassTimeDerivative";
  std::string kernel_name = prefix() + "wcns_mass_time";

  if (_porous_medium_treatment)
  {
    mass_kernel_type = "PWCNSFVMassTimeDerivative";
    kernel_name = prefix() + "pwcns_mass_time";
  }

  InputParameters params = getFactory().getValidParams(mass_kernel_type);
  assignBlocks(params, blocks());
  params.set<NonlinearVariableName>("variable") = _pressure_name;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  getProblem().addFVKernel(mass_kernel_type, kernel_name, params);
}

void
WCNSFVFlowPhysics::addINSMassKernels()
{
  std::string kernel_type = "INSFVMassAdvection";
  std::string kernel_name = prefix() + "ins_mass_advection";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMassAdvection";
    kernel_name = prefix() + "pins_mass_advection";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, blocks());
  params.set<NonlinearVariableName>("variable") = _pressure_name;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("mass_advection_interpolation");

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVFlowPhysics::addINSMomentumGravityKernels()
{
  if (parameters().isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumGravity";
    std::string kernel_name = prefix() + "ins_momentum_gravity_";
    std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumGravity";
      kernel_name = prefix() + "pins_momentum_gravity_";
      rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
    }

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
    if (_porous_medium_treatment)
      params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int d = 0; d < _dim; ++d)
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
WCNSFVFlowPhysics::addINSMomentumBoussinesqKernels()
{
  if (_compressibility == "weakly-compressible")
    paramError("boussinesq_approximation",
               "We cannot use boussinesq approximation while running in weakly-compressible mode!");

  if (parameters().isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumBoussinesq";
    std::string kernel_name = prefix() + "ins_momentum_boussinesq_";
    std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumBoussinesq";
      kernel_name = prefix() + "pins_momentum_boussinesq_";
      rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
    }

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
    params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
    params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
    params.set<MooseFunctorName>("alpha_name") = getParam<MooseFunctorName>("thermal_expansion");
    if (_porous_medium_treatment)
      params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      params.set<NonlinearVariableName>("variable") = _velocity_names[d];

      getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
  }
}

void
WCNSFVFlowPhysics::addINSMomentumFrictionKernels()
{
  const auto friction_blocks = getParam<std::vector<std::vector<SubdomainName>>>("friction_blocks");
  const auto friction_types = getParam<std::vector<std::vector<std::string>>>("friction_types");
  const auto friction_coeffs = getParam<std::vector<std::vector<std::string>>>("friction_coeffs");

  unsigned int num_friction_blocks = friction_blocks.size();
  unsigned int num_used_blocks = num_friction_blocks ? num_friction_blocks : 1;

  if (_porous_medium_treatment)
  {
    const std::string kernel_type = "PINSFVMomentumFriction";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<UserObjectName>("rhie_chow_user_object") = prefix() + "pins_rhie_chow_interpolator";
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
    {
      std::string block_name = "";
      if (num_friction_blocks)
      {
        params.set<std::vector<SubdomainName>>("block") = friction_blocks[block_i];
        block_name = Moose::stringify(friction_blocks[block_i]);
      }
      else
      {
        assignBlocks(params, _blocks);
        block_name = std::to_string(block_i);
      }

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
            params.set<MooseFunctorName>("Darcy_name") = friction_coeffs[block_i][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.set<MooseFunctorName>("Forchheimer_name") = friction_coeffs[block_i][type_i];
        }

        getProblem().addFVKernel(kernel_type,
                                 prefix() + "momentum_friction_" + block_name + "_" +
                                     NS::directions[d],
                                 params);
      }

      if (getParam<bool>("use_friction_correction"))
      {
        const std::string correction_kernel_type = "PINSFVMomentumFrictionCorrection";
        InputParameters corr_params = getFactory().getValidParams(correction_kernel_type);
        if (num_friction_blocks)
          corr_params.set<std::vector<SubdomainName>>("block") = friction_blocks[block_i];
        else
          assignBlocks(corr_params, _blocks);
        corr_params.set<MooseFunctorName>(NS::density) = _density_name;
        corr_params.set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "pins_rhie_chow_interpolator";
        corr_params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
        corr_params.set<Real>("consistent_scaling") = getParam<Real>("consistent_scaling");
        for (unsigned int d = 0; d < _dim; ++d)
        {
          corr_params.set<NonlinearVariableName>("variable") = _velocity_names[d];
          corr_params.set<MooseEnum>("momentum_component") = NS::directions[d];
          for (unsigned int type_i = 0; type_i < friction_types[block_i].size(); ++type_i)
          {
            const auto upper_name = MooseUtils::toUpper(friction_types[block_i][type_i]);
            if (upper_name == "DARCY")
              corr_params.set<MooseFunctorName>("Darcy_name") = friction_coeffs[block_i][type_i];
            else if (upper_name == "FORCHHEIMER")
              corr_params.set<MooseFunctorName>("Forchheimer_name") =
                  friction_coeffs[block_i][type_i];
          }

          getProblem().addFVKernel(correction_kernel_type,
                                   prefix() + "pins_momentum_friction_correction_" + block_name +
                                       "_" + NS::directions[d],
                                   corr_params);
        }
      }
    }
  }
  else
  {
    const std::string kernel_type = "INSFVMomentumFriction";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.set<UserObjectName>("rhie_chow_user_object") = prefix() + "ins_rhie_chow_interpolator";

    for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
    {
      std::string block_name = "";
      if (num_friction_blocks)
      {
        params.set<std::vector<SubdomainName>>("block") = friction_blocks[block_i];
        block_name = Moose::stringify(friction_blocks[block_i]);
      }
      else
      {
        assignBlocks(params, _blocks);
        block_name = std::to_string(block_i);
      }

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
            params.set<MooseFunctorName>("linear_coef_name") = friction_coeffs[block_i][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.set<MooseFunctorName>("quadratic_coef_name") = friction_coeffs[block_i][type_i];
        }

        getProblem().addFVKernel(kernel_type,
                                 prefix() + "ins_momentum_friction_" + block_name + "_" +
                                     NS::directions[d],
                                 params);
      }
    }
  }
}

void
WCNSFVFlowPhysics::addFVBCs()
{
  addINSInletBC();
  addINSOutletBC();
  addINSWallsBC();
}

void
WCNSFVFlowPhysics::addINSInletBC()
{
  const auto momentum_inlet_types = getParam<std::vector<MooseEnum>>("momentum_inlet_types");
  const auto momentum_inlet_functions =
      getParam<std::vector<std::vector<FunctionName>>>("momentum_inlet_function");

  unsigned int flux_bc_counter = 0;
  unsigned int velocity_pressure_counter = 0;
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
  {
    if (momentum_inlet_types[bc_ind] == "fixed-velocity")
    {
      const std::string bc_type = "INSFVInletVelocityBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<FunctionName>("function") =
            momentum_inlet_functions[velocity_pressure_counter][d];

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + _inlet_boundaries[bc_ind], params);
      }
      ++velocity_pressure_counter;
    }
    else if (momentum_inlet_types[bc_ind] == "fixed-pressure")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<FunctionName>("function") = momentum_inlet_functions[velocity_pressure_counter][0];
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      getProblem().addFVBC(bc_type, _pressure_name + "_" + _inlet_boundaries[bc_ind], params);
      ++velocity_pressure_counter;
    }
    else if (momentum_inlet_types[bc_ind] == "flux-mass" ||
             momentum_inlet_types[bc_ind] == "flux-velocity")
    {
      const auto flux_inlet_pps = getParam<std::vector<PostprocessorName>>("flux_inlet_pps");
      const auto flux_inlet_directions = getParam<std::vector<Point>>("flux_inlet_directions");

      {
        const std::string bc_type =
            _porous_medium_treatment ? "PWCNSFVMomentumFluxBC" : "WCNSFVMomentumFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);

        if (flux_inlet_directions.size())
          params.set<Point>("direction") = flux_inlet_directions[flux_bc_counter];

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
        if (_porous_medium_treatment)
        {
          params.set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "pins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
        }
        else
          params.set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "ins_rhie_chow_interpolator";

        if (momentum_inlet_types[bc_ind] == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
        }
        else
          params.set<PostprocessorName>("velocity_pp") = flux_inlet_pps[flux_bc_counter];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<NonlinearVariableName>("variable") = _velocity_names[d];

          getProblem().addFVBC(
              bc_type, _velocity_names[d] + "_" + _inlet_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "WCNSFVMassFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<NonlinearVariableName>("variable") = _pressure_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        if (flux_inlet_directions.size())
          params.set<Point>("direction") = flux_inlet_directions[flux_bc_counter];

        if (momentum_inlet_types[bc_ind] == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
        }
        else
          params.set<PostprocessorName>("velocity_pp") = flux_inlet_pps[flux_bc_counter];

        getProblem().addFVBC(bc_type, _pressure_name + "_" + _inlet_boundaries[bc_ind], params);
      }

      // need to increment flux_bc_counter
      ++flux_bc_counter;
    }
  }
}

void
WCNSFVFlowPhysics::addINSOutletBC()
{
  const auto pressure_functions = getParam<std::vector<FunctionName>>("pressure_function");
  const auto momentum_outlet_types = getParam<std::vector<MooseEnum>>("pressure_function");

  const std::string u_names[3] = {"u", "v", "w"};
  for (unsigned int bc_ind = 0; bc_ind < _outlet_boundaries.size(); ++bc_ind)
  {

    if (momentum_outlet_types[bc_ind] == "zero-gradient" ||
        momentum_outlet_types[bc_ind] == "fixed-pressure-zero-gradient")
    {
      if (_porous_medium_treatment)
      {
        const std::string bc_type = "PINSFVMomentumAdvectionOutflowBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
        params.set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "pins_rhie_chow_interpolator";
        params.set<MooseFunctorName>(NS::density) = _density_name;

        for (unsigned int i = 0; i < _dim; ++i)
          params.set<MooseFunctorName>(u_names[i]) = _velocity_names[i];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<NonlinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(
              bc_type, _velocity_names[d] + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
      else
      {
        const std::string bc_type = "INSFVMomentumAdvectionOutflowBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        params.set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "ins_rhie_chow_interpolator";
        params.set<MooseFunctorName>(NS::density) = _density_name;

        for (unsigned int i = 0; i < _dim; ++i)
          params.set<MooseFunctorName>(u_names[i]) = _velocity_names[i];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<NonlinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(
              bc_type, _velocity_names[d] + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
    }

    if (momentum_outlet_types[bc_ind] == "fixed-pressure" ||
        momentum_outlet_types[bc_ind] == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<FunctionName>("function") = pressure_functions[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

      getProblem().addFVBC(bc_type, _pressure_name + "_" + _outlet_boundaries[bc_ind], params);
    }
    else if (momentum_outlet_types[bc_ind] == "zero-gradient")
    {
      const std::string bc_type = "INSFVMassAdvectionOutflowBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
        params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

      getProblem().addFVBC(bc_type, _pressure_name + "_" + _outlet_boundaries[bc_ind], params);
    }
  }
}

void
WCNSFVFlowPhysics::addINSWallsBC()
{
  const auto momentum_wall_types = getParam<std::vector<MooseEnum>>("momentum_wall_types");
  const std::string u_names[3] = {"u", "v", "w"};

  for (unsigned int bc_ind = 0; bc_ind < _wall_boundaries.size(); ++bc_ind)
  {
    if (momentum_wall_types[bc_ind] == "noslip")
    {
      const std::string bc_type = "INSFVNoSlipWallBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<FunctionName>("function") = "0";

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (momentum_wall_types[bc_ind] == "wallfunction")
    {
      const std::string bc_type = "INSFVWallFunctionBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      if (_porous_medium_treatment)
        params.set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "pins_rhie_chow_interpolator";
      else
        params.set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "ins_rhie_chow_interpolator";

      for (unsigned int d = 0; d < _dim; ++d)
        params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (momentum_wall_types[bc_ind] == "slip")
    {
      const std::string bc_type = "INSFVNaturalFreeSlipBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        if (_porous_medium_treatment)
          params.set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "pins_rhie_chow_interpolator";
        else
          params.set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "ins_rhie_chow_interpolator";

        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (momentum_wall_types[bc_ind] == "symmetry")
    {
      {
        std::string bc_type;
        if (_porous_medium_treatment)
          bc_type = "PINSFVSymmetryVelocityBC";
        else
          bc_type = "INSFVSymmetryVelocityBC";

        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        MooseFunctorName viscosity_name = _dynamic_viscosity_name;
        if (hasTurbulencePhysics())
          viscosity_name = NS::total_viscosity;
        params.set<MooseFunctorName>(NS::mu) = viscosity_name;

        for (unsigned int d = 0; d < _dim; ++d)
          params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          if (_porous_medium_treatment)
            params.set<UserObjectName>("rhie_chow_user_object") =
                prefix() + "pins_rhie_chow_interpolator";
          else
            params.set<UserObjectName>("rhie_chow_user_object") =
                prefix() + "ins_rhie_chow_interpolator";

          params.set<NonlinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(
              bc_type, _velocity_names[d] + "_" + _wall_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "INSFVSymmetryPressureBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _pressure_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        getProblem().addFVBC(bc_type, _pressure_name + "_" + _wall_boundaries[bc_ind], params);
      }
    }
  }
}

void
WCNSFVFlowPhysics::addUserObjects()
{
  // Rhie Chow user object for interpolation velocities
  WCNSFVPhysicsBase::addUserObjects();

  // Pressure pin
  if (getParam<bool>("pin_pressure"))
  {
    MooseEnum pin_type = getParam<MooseEnum>("pinned_pressure_type");
    std::string object_type = "NSFVPressurePin";

    // Create the average value postprocessor if needed
    if (pin_type == "average-uo")
    {
      // Volume average by default, but we could do inlet or outlet for example
      InputParameters params = getFactory().getValidParams("ElementAverageValue");
      params.set<std::vector<VariableName>>("variable") = {_pressure_name};
      assignBlocks(params, blocks());
      params.set<std::vector<OutputName>>("outputs") = {"none"};
      getProblem().addPostprocessor("ElementAverageValue", "nsfv_pressure_average", params);
    }

    InputParameters params = getFactory().getValidParams(object_type);
    if (pin_type == "point-value-uo")
      params.set<MooseEnum>("pin_type") = "point-value";
    else
      params.set<MooseEnum>("pin_type") = "average";

    params.set<PostprocessorName>("phi0") = getParam<PostprocessorName>("pinned_pressure_value");
    params.set<NonlinearVariableName>("variable") = _pressure_name;
    if (pin_type == "point-value" || pin_type == "point-value-uo")
      params.set<Point>("point") = getParam<Point>("pinned_pressure_point");
    else if (pin_type == "average-uo")
      params.set<PostprocessorName>("pressure_average") = "nsfv_pressure_average";

    getProblem().addUserObject(object_type, prefix() + "ins_mass_pressure_pin", params);
  }
}
