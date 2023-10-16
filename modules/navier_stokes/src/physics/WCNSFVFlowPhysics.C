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

registerMooseObject("NavierStokesApp", WCNSFVFlowPhysics);

InputParameters
WCNSFVFlowPhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible mass and momentum equations");

  params += NSFVAction::commonMomentumEquationParams();
  params.transferParam<std::vector<FunctionName>>(NSFVAction::validParams(), "pressure_function");

  // Initialization parameters
  params.transferParam<std::vector<FunctionName>>(NSFVAction::validParams(), "initial_velocity");
  params.transferParam<FunctionName>(NSFVAction::validParams(), "initial_pressure");

  // Techniques to limit or remove oscillations at porosity jump interfaces
  params.transferParam<MooseEnum>(NSFVAction::validParams(),
                                  "porosity_interface_pressure_treatment");

  // Friction correction, a technique to limit oscillations at friction interfaces
  params.transferParam<bool>(NSFVAction::validParams(), "use_friction_correction");
  params.transferParam<Real>(NSFVAction::validParams(), "consistent_scaling");

  // Spatial discretization scheme
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "mass_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "momentum_advection_interpolation");
  params.transferParam<bool>(NSFVAction::validParams(), "pressure_two_term_bc_expansion");
  params.transferParam<bool>(NSFVAction::validParams(), "momentum_two_term_bc_expansion");

  // Nonlinear solver parameters
  params.transferParam<Real>(NSFVAction::validParams(), "mass_scaling");
  params.transferParam<Real>(NSFVAction::validParams(), "momentum_scaling");

  return params;
}

WCNSFVFlowPhysics::WCNSFVFlowPhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters)
{
  checkTwoDVectorParamsSameLength<SubdomainName, std::string>("friction_blocks", "friction_types");
  checkTwoDVectorParamsSameLength<SubdomainName, std::string>("friction_blocks", "friction_coeffs");

  checkSecondParamSetOnlyIfFirstOneTrue("pin_pressure", "pinned_pressure_type");
  checkSecondParamSetOnlyIfFirstOneTrue("pin_pressure", "pinned_pressure_value");
  if (std::string(getParam<MooseEnum>("pinned_pressure_type")).find("point") != std::string::npos &&
      !isParamValid("pinned_pressure_point"))
    paramError("pinned_pressure_point",
               "This parameter must be set to specify the pinned pressure point");

  checkSecondParamSetOnlyIfFirstOneTrue("use_friction_correction", "consistent_scaling");
  checkSecondParamSetOnlyIfFirstOneTrue("boussinesq_approximation", "ref_temperature");
  checkSecondParamSetOnlyIfFirstOneTrue("boussinesq_approximation", "thermal_expansion");

  checkSecondParamSetOnlyIfFirstOneTrue("porous_medium_treatment",
                                        "porosity_interface_pressure_treatment");
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
    assignBlocks(params, _blocks); // TODO: check wrt components
    params.set<std::vector<Real>>("scaling") = {getParam<Real>("momentum_scaling")};
    params.set<MooseEnum>("face_interp_method") =
        getParam<MooseEnum>("momentum_advection_interpolation");
    params.set<bool>("two_term_boundary_expansion") =
        getParam<bool>("momentum_two_term_bc_expansion");

    for (unsigned int d = 0; d < _dim; ++d)
    {
      if (_verbose)
        _console << "Creating variable " << _velocity_names[d] << " on blocks "
                 << Moose::stringify(_blocks) << std::endl;
      getProblem().addVariable(variable_type, _velocity_names[d], params);
    }
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
    assignBlocks(params, _blocks);
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

    if (_verbose)
      _console << "Creating variable " << _pressure_name << " on blocks "
               << Moose::stringify(_blocks) << std::endl;
    getProblem().addVariable(pressure_type, _pressure_name, params);
  }
}

void
WCNSFVFlowPhysics::addFVKernels()
{
  // Mass equation: time derivative
  if (_compressibility == "weakly-compressible" && isTransient())
    addWCNSMassTimeKernels();

  // Mass equation: divergence of momentum
  addINSMassKernels();

  // Momentum equation: time derivative
  if (isTransient())
  {
    if (_compressibility == "incompressible")
      addINSMomentumTimeKernels();
    else
      addWCNSMomentumTimeKernels();
  }

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
  assignBlocks(params, _blocks);
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

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMassAdvection";
    kernel_name = prefix() + "pins_mass_advection";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _pressure_name;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("mass_advection_interpolation");

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

void
WCNSFVFlowPhysics::addINSMomentumTimeKernels()
{
  std::string kernel_type = "INSFVMomentumTimeDerivative";
  std::string kernel_name = prefix() + "ins_momentum_time_";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumTimeDerivative";
    kernel_name = prefix() + "pins_momentum_time_";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addFVKernel(kernel_type, kernel_name + _velocity_names[d], params);
  }
}

void
WCNSFVFlowPhysics::addWCNSMomentumTimeKernels()
{
  const std::string mom_kernel_type = "WCNSFVMomentumTimeDerivative";
  InputParameters params = getFactory().getValidParams(mom_kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

    if (_porous_medium_treatment)
      getProblem().addFVKernel(
          mom_kernel_type, prefix() + "pwcns_momentum_" + NS::directions[d] + "_time", params);
    else
      getProblem().addFVKernel(
          mom_kernel_type, prefix() + "wcns_momentum_" + NS::directions[d] + "_time", params);
  }
}

void
WCNSFVFlowPhysics::addINSMomentumAdvectionKernels()
{
  std::string kernel_type = "INSFVMomentumAdvection";
  std::string kernel_name = prefix() + "ins_momentum_advection_";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumAdvection";
    kernel_name = prefix() + "pins_momentum_advection_";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseEnum>("advected_interp_method") =
      getParam<MooseEnum>("momentum_advection_interpolation");
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
  params.applySpecificParameters(parameters(), INSFVMomentumAdvection::listOfCommonParams());

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSFVFlowPhysics::addINSMomentumViscousDissipationKernels()
{
  std::string kernel_type = "INSFVMomentumDiffusion";
  std::string kernel_name = prefix() + "ins_momentum_diffusion_";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumDiffusion";
    kernel_name = prefix() + "pins_momentum_diffusion_";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSFVFlowPhysics::addINSMomentumPressureKernels()
{
  std::string kernel_type = "INSFVMomentumPressure";
  std::string kernel_name = prefix() + "ins_momentum_pressure_";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumPressure";
    kernel_name = prefix() + "pins_momentum_pressure_";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseFunctorName>("pressure") = _pressure_name;
  params.set<bool>("correct_skewness") =
      getParam<MooseEnum>("pressure_face_interpolation") == "skewness-corrected";
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSFVFlowPhysics::addINSMomentumGravityKernels()
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

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumBoussinesq";
      kernel_name = prefix() + "pins_momentum_boussinesq_";
    }

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
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
    params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
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
        corr_params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
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
    params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

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
  const auto momentum_inlet_types = getParam<MultiMooseEnum>("momentum_inlet_types");
  const auto momentum_inlet_functions =
      getParam<std::vector<std::vector<FunctionName>>>("momentum_inlet_function");

  unsigned int flux_bc_counter = 0;
  unsigned int velocity_pressure_counter = 0;
  for (unsigned int bc_ind = 0; bc_ind < momentum_inlet_types.size(); ++bc_ind)
  {
    if (momentum_inlet_types[bc_ind] == "fixed-velocity")
    {
      const std::string bc_type = "INSFVInletVelocityBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseFunctorName>("functor") =
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
        params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
        if (_porous_medium_treatment)
          params.set<MooseFunctorName>(NS::porosity) = _porosity_name;

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
  const auto momentum_outlet_types = getParam<MultiMooseEnum>("momentum_outlet_types");

  const std::string u_names[3] = {"u", "v", "w"};
  for (unsigned int bc_ind = 0; bc_ind < momentum_outlet_types.size(); ++bc_ind)
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
        params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
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
        params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
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
  const auto momentum_wall_types = getParam<MultiMooseEnum>("momentum_wall_types");
  const std::string u_names[3] = {"u", "v", "w"};

  for (unsigned int bc_ind = 0; bc_ind < momentum_wall_types.size(); ++bc_ind)
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
      params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

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
      params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

      for (unsigned int d = 0; d < _dim; ++d)
      {
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
        params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

        for (unsigned int d = 0; d < _dim; ++d)
          params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

        for (unsigned int d = 0; d < _dim; ++d)
        {
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
      assignBlocks(params, _blocks);
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

void
WCNSFVFlowPhysics::addMaterials()
{
  if (_porous_medium_treatment)
    addPorousMediumSpeedMaterial();
}

void
WCNSFVFlowPhysics::addPorousMediumSpeedMaterial()
{
  InputParameters params = getFactory().getValidParams("PINSFVSpeedFunctorMaterial");
  assignBlocks(params, _blocks);

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.set<MooseFunctorName>(NS::superficial_velocity_vector[dim_i]) = _velocity_names[dim_i];
  params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  getProblem().addMaterial("PINSFVSpeedFunctorMaterial", prefix() + "pins_speed_material", params);
}

void
WCNSFVFlowPhysics::addInitialConditions()
{
  // do not set initial conditions if we load from file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);
  auto vvalue = getParam<std::vector<FunctionName>>("initial_velocity");

  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_velocity"))
    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<VariableName>("variable") = _velocity_names[d];
      params.set<FunctionName>("function") = vvalue[d];

      getProblem().addInitialCondition("FunctionIC", prefix() + _velocity_names[d] + "_ic", params);
      // addNSInitialCondition("FunctionIC", prefix() + _velocity_name[d] + "_ic", params);
    }

  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_pressure"))
  {
    params.set<VariableName>("variable") = _pressure_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_pressure");

    getProblem().addInitialCondition("FunctionIC", prefix() + _pressure_name + "_ic", params);
    // addNSInitialCondition("FunctionIC", prefix() + _pressure_name + "_ic", params);
  }

  // if (_has_scalar_equation &&
  //     (!_app.isRestarting() || parameters().isParamSetByUser("initial_scalar_variables")))
  // {
  //   unsigned int ic_counter = 0;
  //   for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  //   {
  //     bool initialize_me = true;
  //     if (_create_scalar_variable.size())
  //       if (!_create_scalar_variable[name_i])
  //         initialize_me = false;

  //     if (initialize_me)
  //     {
  //       params.set<VariableName>("variable") = _passive_scalar_names[name_i];
  //       if (parameters().isParamValid("initial_scalar_variables"))
  //         params.set<FunctionName>("function") =
  //             getParam<std::vector<FunctionName>>("initial_scalar_variables")[ic_counter];
  //       else
  //         params.set<FunctionName>("function") = "0.0";

  //       addNSInitialCondition(
  //           "FunctionIC", prefix() + _passive_scalar_names[name_i] + "_ic", params);
  //       ic_counter += 1;
  //     }
  //   }
  // }
}

unsigned short
WCNSFVFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  unsigned short ghost_layers = WCNSFVPhysicsBase::getNumberAlgebraicGhostingLayersNeeded();
  if (_porous_medium_treatment &&
      getParam<MooseEnum>("porosity_interface_pressure_treatment") != "automatic")
    ghost_layers = std::max(ghost_layers, (unsigned short)3);
  return ghost_layers;
}
