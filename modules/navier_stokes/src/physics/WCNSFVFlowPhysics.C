//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVFlowPhysics.h"
#include "WCNSFVTurbulencePhysics.h"
#include "NSFVBase.h"
#include "INSFVMomentumAdvection.h"
#include "INSFVRhieChowInterpolator.h"
#include "INSFVTimeKernel.h"
#include "MapConversionUtils.h"
#include "NS.h"

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp", WCNSFVFlowPhysics);
registerMooseAction("NavierStokesApp", WCNSFVFlowPhysics, "add_fv_kernel");
registerMooseAction("NavierStokesApp", WCNSFVFlowPhysics, "add_fv_bc");

InputParameters
WCNSFVFlowPhysics::validParams()
{
  InputParameters params = WCNSFVFlowPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible mass and momentum equations");

  // Rhie Chow interpolation parameters
  params.transferParam<Real>(INSFVMomentumAdvection::validParams(), "characteristic_speed");
  params.addParam<bool>(
      "time_derivative_contributes_to_RC_coefficients",
      true,
      "Whether the time derivative term should contribute to the Rhie Chow coefficients. This adds "
      "stabilization, but makes the solution dependent on the time step size");
  params.addParamNamesToGroup("time_derivative_contributes_to_RC_coefficients characteristic_speed",
                              "Numerical scheme");

  // Used for flow mixtures, where one phase is solid / not moving under the action of gravity
  params.addParam<MooseFunctorName>(
      "density_for_gravity_terms",
      "If specified, replaces the 'density' for the Boussinesq and gravity momentum kernels");

  // Additional porous media parameters
  params.transferParam<unsigned short>(NSFVBase::validParams(), "porosity_smoothing_layers");

  // Techniques to limit or remove oscillations at porosity jump interfaces
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "porosity_interface_pressure_treatment");

  // Friction correction, a technique to limit oscillations at friction interfaces
  params.transferParam<bool>(NSFVBase::validParams(), "use_friction_correction");
  params.transferParam<Real>(NSFVBase::validParams(), "consistent_scaling");

  // Couple to turbulence physics
  params.addParam<PhysicsName>("coupled_turbulence_physics",
                               "Turbulence Physics coupled with the flow");

  // Spatial discretization scheme
  // Specify the numerical schemes for interpolations of velocity and pressure
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "pressure_face_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "momentum_face_interpolation");
  params.transferParam<MooseEnum>(NSFVBase::validParams(), "mass_advection_interpolation");
  params.transferParam<bool>(NSFVBase::validParams(),
                             "pressure_allow_expansion_on_bernoulli_faces");

  // Nonlinear solver parameters
  params.transferParam<Real>(NSFVBase::validParams(), "mass_scaling");
  params.transferParam<Real>(NSFVBase::validParams(), "momentum_scaling");

  // Parameter groups
  params.addParamNamesToGroup("coupled_turbulence_physics", "Coupled Physics");
  params.addParamNamesToGroup(
      "porosity_interface_pressure_treatment pressure_allow_expansion_on_bernoulli_faces "
      "porosity_smoothing_layers use_friction_correction consistent_scaling",
      "Flow medium discontinuity treatment");
  params.addParamNamesToGroup("pressure_face_interpolation momentum_face_interpolation "
                              "mass_advection_interpolation momentum_advection_interpolation "
                              "mass_scaling momentum_scaling characteristic_speed",
                              "Numerical scheme");

  // TODO Add default preconditioning and move scaling parameters to a preconditioning group

  return params;
}

WCNSFVFlowPhysics::WCNSFVFlowPhysics(const InputParameters & parameters)
  : WCNSFVFlowPhysicsBase(parameters),
    _porosity_smoothing_layers(isParamValid("porosity_smoothing_layers")
                                   ? getParam<unsigned short>("porosity_smoothing_layers")
                                   : 0),
    _friction_blocks(getParam<std::vector<std::vector<SubdomainName>>>("friction_blocks")),
    _friction_types(getParam<std::vector<std::vector<std::string>>>("friction_types")),
    _friction_coeffs(getParam<std::vector<std::vector<std::string>>>("friction_coeffs"))
{
  _flow_porosity_functor_name = isParamValid("porosity_smoothing_layers") &&
                                        getParam<unsigned short>("porosity_smoothing_layers")
                                    ? NS::smoothed_porosity
                                    : _porosity_name;

  // Most likely to be a mistake
  if (getParam<bool>("pin_pressure") &&
      getParam<std::vector<MooseFunctorName>>("pressure_functors").size())
    paramError("pin_pressure", "Cannot pin the pressure if a pressure boundary exists");

  // Friction parameter checks
  if (_friction_blocks.size())
    checkVectorParamsSameLength<std::vector<SubdomainName>, std::vector<std::string>>(
        "friction_blocks", "friction_types");
  checkTwoDVectorParamsSameLength<std::string, std::string>("friction_types", "friction_coeffs");

  // Pressure pin checks
  checkSecondParamSetOnlyIfFirstOneTrue("pin_pressure", "pinned_pressure_type");
  checkSecondParamSetOnlyIfFirstOneTrue("pin_pressure", "pinned_pressure_value");
  if (getParam<bool>("pin_pressure"))
  {
    if ((std::string(getParam<MooseEnum>("pinned_pressure_type")).find("point") !=
         std::string::npos) &&
        !isParamSetByUser("pinned_pressure_point"))
      paramError("pinned_pressure_point",
                 "This parameter must be set to specify the pinned pressure point");
    else if ((std::string(getParam<MooseEnum>("pinned_pressure_type")).find("point") ==
              std::string::npos) &&
             isParamSetByUser("pinned_pressure_point"))
      paramError("pinned_pressure_point",
                 "This parameter should not be given by the user with the corresponding "
                 "pinned_pressure_type setting: " +
                     std::string(getParam<MooseEnum>("pinned_pressure_type")) + ".");
  }

  // Porosity correction checks
  checkSecondParamSetOnlyIfFirstOneTrue("porous_medium_treatment", "use_friction_correction");
  checkSecondParamSetOnlyIfFirstOneTrue("use_friction_correction", "consistent_scaling");
  checkSecondParamSetOnlyIfFirstOneTrue("porous_medium_treatment",
                                        "porosity_interface_pressure_treatment");
  if (getParam<MooseEnum>("porosity_interface_pressure_treatment") != "bernoulli")
    errorDependentParameter("porosity_interface_pressure_treatment",
                            "bernoulli",
                            {"pressure_allow_expansion_on_bernoulli_faces"});

  // Porous media parameters
  checkSecondParamSetOnlyIfFirstOneTrue("porous_medium_treatment", "porosity_smoothing_layers");
}

void
WCNSFVFlowPhysics::addSolverVariables()
{
  if (!_has_flow_equations)
    return;

  for (const auto d : make_range(dimension()))
    saveSolverVariableName(_velocity_names[d]);
  saveSolverVariableName(_pressure_name);

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
    if (variableExists(_velocity_names[d], true))
      checkBlockRestrictionIdentical(_velocity_names[d],
                                     getProblem().getVariable(0, _velocity_names[d]).blocks());
    else if (_define_variables)
    {
      std::string variable_type = "INSFVVelocityVariable";
      if (_porous_medium_treatment)
        variable_type = "PINSFVSuperficialVelocityVariable";

      auto params = getFactory().getValidParams(variable_type);
      assignBlocks(params, _blocks); // TODO: check wrt components
      params.set<std::vector<Real>>("scaling") = {getParam<Real>("momentum_scaling")};
      params.set<MooseEnum>("face_interp_method") =
          getParam<MooseEnum>("momentum_face_interpolation");
      params.set<bool>("two_term_boundary_expansion") =
          getParam<bool>("momentum_two_term_bc_expansion");

      for (const auto d : make_range(dimension()))
      {
        params.set<SolverSystemName>("solver_sys") = getSolverSystem(_velocity_names[d]);
        getProblem().addVariable(variable_type, _velocity_names[d], params);
      }
    }
    else
      paramError("velocity_variable",
                 "Variable (" + _velocity_names[d] +
                     ") supplied to the WCNSFVFlowPhysics does not exist!");
  }

  // Pressure
  if (variableExists(_pressure_name, true))
    checkBlockRestrictionIdentical(_pressure_name,
                                   getProblem().getVariable(0, _pressure_name).blocks());
  else if (_define_variables)
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
      if (dimension() >= 2)
        params.set<MooseFunctorName>("v") = _velocity_names[1];
      if (dimension() == 3)
        params.set<MooseFunctorName>("w") = _velocity_names[2];
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<bool>("allow_two_term_expansion_on_bernoulli_faces") =
          getParam<bool>("pressure_allow_expansion_on_bernoulli_faces");
    }
    getProblem().addVariable(pressure_type, _pressure_name, params);
  }
  else
    paramError("pressure_variable",
               "Variable (" + _pressure_name +
                   ") supplied to the WCNSFVFlowPhysics does not exist!");

  // Add lagrange multiplier for pinning pressure, if needed
  if (getParam<bool>("pin_pressure"))
  {
    auto type = getParam<MooseEnum>("pinned_pressure_type");
    auto lm_params = getFactory().getValidParams("MooseVariableScalar");
    lm_params.set<MooseEnum>("family") = "scalar";
    lm_params.set<MooseEnum>("order") = "first";

    if (type == "point-value" || type == "average")
      getProblem().addVariable("MooseVariableScalar", "lambda", lm_params);
  }
}

void
WCNSFVFlowPhysics::addFVKernels()
{
  if (!_has_flow_equations)
    return;

  // Mass equation: time derivative
  if (_compressibility == "weakly-compressible" && isTransient())
    addWCNSMassTimeKernels();

  // Mass equation: divergence of momentum
  addINSMassKernels();

  // Pressure pin
  if (getParam<bool>("pin_pressure"))
    addINSPressurePinKernel();

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
  if (_friction_types.size())
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
WCNSFVFlowPhysics::addINSPressurePinKernel()
{
  const auto pin_type = getParam<MooseEnum>("pinned_pressure_type");
  const auto object_type =
      (pin_type == "average") ? "FVIntegralValueConstraint" : "FVPointValueConstraint";
  InputParameters params = getFactory().getValidParams(object_type);
  if (pin_type != "point-value" && pin_type != "average")
    return;

  params.set<CoupledName>("lambda") = {"lambda"};
  params.set<PostprocessorName>("phi0") = getParam<PostprocessorName>("pinned_pressure_value");
  params.set<NonlinearVariableName>("variable") = _pressure_name;
  if (pin_type == "point-value")
    params.set<Point>("point") = getParam<Point>("pinned_pressure_point");

  getProblem().addFVKernel(object_type, prefix() + "ins_mass_pressure_pin", params);
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
  params.set<bool>("contribute_to_rc") =
      getParam<bool>("time_derivative_contributes_to_RC_coefficients");

  for (const auto d : make_range(dimension()))
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
  params.set<bool>("contribute_to_rc") =
      getParam<bool>("time_derivative_contributes_to_RC_coefficients");

  for (const auto d : make_range(dimension()))
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
  params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
  params.applySpecificParameters(parameters(), INSFVMomentumAdvection::listOfCommonParams());

  for (const auto d : make_range(dimension()))
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

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumDiffusion";
    kernel_name = prefix() + "pins_momentum_diffusion_";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
  params.set<MooseEnum>("mu_interp_method") = getParam<MooseEnum>("mu_interp_method");
  if (getParam<bool>("include_deviatoric_stress"))
  {
    params.set<bool>("complete_expansion") = true;
    const std::string u_names[3] = {"u", "v", "w"};
    for (unsigned int i = 0; i < dimension(); ++i)
      params.set<MooseFunctorName>(u_names[i]) = _velocity_names[i];
  }

  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
  // Currently only Newton method for WCNSFVFlowPhysics
  params.set<bool>("newton_solve") = true;
  for (const auto d : make_range(dimension()))
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

  for (const auto d : make_range(dimension()))
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<NonlinearVariableName>("variable") = _velocity_names[d];
    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
WCNSFVFlowPhysics::addINSMomentumGravityKernels()
{
  if (parameters().isParamValid("gravity") && !_solve_for_dynamic_pressure)
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
WCNSFVFlowPhysics::addINSMomentumFrictionKernels()
{
  unsigned int num_friction_blocks = _friction_blocks.size();
  unsigned int num_used_blocks = num_friction_blocks ? num_friction_blocks : 1;

  const std::string kernel_type = "PINSFVMomentumFriction";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
  if (hasForchheimerFriction())
    params.set<MooseFunctorName>(NS::speed) = NS::speed;
  params.set<bool>("standard_friction_formulation") =
      getParam<bool>("standard_friction_formulation");
  params.set<bool>("is_porous_medium") = _porous_medium_treatment;

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
      params.set<NonlinearVariableName>("variable") = _velocity_names[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
      {
        const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
        if (upper_name == "DARCY")
        {
          params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
          params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
        }
        else if (upper_name == "FORCHHEIMER")
        {
          params.set<MooseFunctorName>(NS::speed) = NS::speed;
          params.set<MooseFunctorName>("Forchheimer_name") = _friction_coeffs[block_i][type_i];
        }
      }

      getProblem().addFVKernel(kernel_type,
                               prefix() + "momentum_friction_" + block_name + "_" +
                                   NS::directions[d],
                               params);
    }

    if (_porous_medium_treatment && getParam<bool>("use_friction_correction"))
    {
      const std::string correction_kernel_type = "PINSFVMomentumFrictionCorrection";
      InputParameters corr_params = getFactory().getValidParams(correction_kernel_type);
      if (num_friction_blocks)
        corr_params.set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
      else
        assignBlocks(corr_params, _blocks);
      corr_params.set<MooseFunctorName>(NS::density) = _density_name;
      corr_params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
      corr_params.set<Real>("consistent_scaling") = getParam<Real>("consistent_scaling");
      for (const auto d : make_range(dimension()))
      {
        corr_params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        corr_params.set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
          {
            corr_params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
            corr_params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
          }
          else if (upper_name == "FORCHHEIMER")
          {
            corr_params.set<MooseFunctorName>(NS::speed) = NS::speed;
            corr_params.set<MooseFunctorName>("Forchheimer_name") =
                _friction_coeffs[block_i][type_i];
          }
        }

        getProblem().addFVKernel(correction_kernel_type,
                                 prefix() + "pins_momentum_friction_correction_" + block_name +
                                     "_" + NS::directions[d],
                                 corr_params);
      }
    }
  }
}

void
WCNSFVFlowPhysics::addINSInletBC()
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

  unsigned int flux_bc_counter = 0;
  unsigned int velocity_pressure_counter = 0;
  for (const auto & [inlet_bdy, momentum_inlet_type] : _momentum_inlet_types)
  {
    if (momentum_inlet_type == "fixed-velocity")
    {
      const std::string bc_type = "INSFVInletVelocityBC";
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

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + inlet_bdy, params);
      }
      ++velocity_pressure_counter;
    }
    else if (momentum_inlet_type == "fixed-pressure")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      if (_momentum_inlet_functors.size() < velocity_pressure_counter + 1)
        paramError("momentum_inlet_functors",
                   "More non-flux inlets than inlet functors (" +
                       std::to_string(_momentum_inlet_functors.size()) + ")");

      params.set<FunctionName>("function") =
          libmesh_map_find(_momentum_inlet_functors, inlet_bdy)[0];
      params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};

      getProblem().addFVBC(bc_type, _pressure_name + "_" + inlet_bdy, params);
      ++velocity_pressure_counter;
    }
    else if (momentum_inlet_type == "flux-mass" || momentum_inlet_type == "flux-velocity")
    {
      {
        const std::string bc_type =
            _porous_medium_treatment ? "PWCNSFVMomentumFluxBC" : "WCNSFVMomentumFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);

        if (_flux_inlet_directions.size())
          params.set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};
        params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
        if (_porous_medium_treatment)
          params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
        if (_flux_inlet_pps.size() < flux_bc_counter + 1)
          paramError("flux_inlet_pps",
                     "More inlet flux BCs than inlet flux pps (" +
                         std::to_string(_flux_inlet_pps.size()) + ")");

        if (momentum_inlet_type == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + inlet_bdy;
        }
        else
          params.set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        for (const auto d : make_range(dimension()))
          params.set<MooseFunctorName>(NS::velocity_vector[d]) = _velocity_names[d];

        for (const auto d : make_range(dimension()))
        {
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<NonlinearVariableName>("variable") = _velocity_names[d];

          getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + inlet_bdy, params);
        }
      }
      {
        const std::string bc_type = "WCNSFVMassFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<NonlinearVariableName>("variable") = _pressure_name;
        params.set<std::vector<BoundaryName>>("boundary") = {inlet_bdy};

        if (_flux_inlet_directions.size())
          params.set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];

        if (momentum_inlet_type == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + inlet_bdy;
        }
        else
          params.set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        for (const auto d : make_range(dimension()))
          params.set<MooseFunctorName>(NS::velocity_vector[d]) = _velocity_names[d];

        getProblem().addFVBC(bc_type, _pressure_name + "_" + inlet_bdy, params);
      }

      // need to increment flux_bc_counter
      ++flux_bc_counter;
    }
  }
}

void
WCNSFVFlowPhysics::addINSOutletBC()
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
    if (momentum_outlet_type == "zero-gradient" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      {
        const std::string bc_type = _porous_medium_treatment ? "PINSFVMomentumAdvectionOutflowBC"
                                                             : "INSFVMomentumAdvectionOutflowBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
        if (_porous_medium_treatment)
          params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
        params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();
        params.set<MooseFunctorName>(NS::density) = _density_name;

        for (unsigned int i = 0; i < dimension(); ++i)
          params.set<MooseFunctorName>(u_names[i]) = _velocity_names[i];

        for (const auto d : make_range(dimension()))
        {
          params.set<NonlinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
        }
      }
    }

    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>("functor") = libmesh_map_find(_pressure_functors, outlet_bdy);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};

      getProblem().addFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
    }
    else if (momentum_outlet_type == "zero-gradient")
    {
      const std::string bc_type = "INSFVMassAdvectionOutflowBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};

      for (const auto d : make_range(dimension()))
        params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

      getProblem().addFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
    }
  }
}

void
WCNSFVFlowPhysics::addINSWallsBC()
{
  const std::string u_names[3] = {"u", "v", "w"};

  // Count the number of fixed velocity wall boundaries (moving walls)
  unsigned int num_functor_walls = 0;
  for (const auto & [boundary_name, momentum_wall_type] : _momentum_wall_types)
    if (momentum_wall_type == "noslip")
      num_functor_walls++;
  if (_momentum_wall_functors.size() && num_functor_walls != _momentum_wall_functors.size())
    paramError("momentum_wall_functors",
               "If any wall functors are specified, the number of boundaries requiring a momentum "
               "functor (" +
                   std::to_string(num_functor_walls) + ") and the number of functors specified (" +
                   std::to_string(_momentum_wall_functors.size()) + ") must match");
  for (const auto & wall_functors : _momentum_wall_functors)
    if (wall_functors.second.size() != dimension())
      paramError("momentum_wall_functors",
                 "Number of wall functors (" + std::to_string(wall_functors.second.size()) +
                     ") must match dimension (" + std::to_string(dimension()) +
                     ").\nFunctors currently specified:" + Moose::stringify(wall_functors.second));

  for (const auto & [boundary_name, wall_type] : _momentum_wall_types)
  {
    if (wall_type == "noslip")
    {
      const std::string bc_type = "INSFVNoSlipWallBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};

      for (const auto d : make_range(dimension()))
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        if (_momentum_wall_functors.count(boundary_name) == 0)
          params.set<FunctionName>("function") = "0";
        else
          params.set<FunctionName>("function") = _momentum_wall_functors[boundary_name][d];

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
      }
    }
    else if (wall_type == "wallfunction")
    {
      const std::string bc_type = "INSFVWallFunctionBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
      params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

      for (const auto d : make_range(dimension()))
        params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

      for (const auto d : make_range(dimension()))
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
      }
    }
    else if (wall_type == "slip")
    {
      const std::string bc_type = "INSFVNaturalFreeSlipBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
      params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

      for (const auto d : make_range(dimension()))
      {
        params.set<NonlinearVariableName>("variable") = _velocity_names[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
      }
    }
    else if (wall_type == "symmetry")
    {
      {
        std::string bc_type;
        if (_porous_medium_treatment)
          bc_type = "PINSFVSymmetryVelocityBC";
        else
          bc_type = "INSFVSymmetryVelocityBC";

        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};

        MooseFunctorName viscosity_name = _dynamic_viscosity_name;
        if (hasTurbulencePhysics())
          viscosity_name = NS::total_viscosity;
        params.set<MooseFunctorName>(NS::mu) = viscosity_name;
        params.set<UserObjectName>("rhie_chow_user_object") = rhieChowUOName();

        for (const auto d : make_range(dimension()))
          params.set<MooseFunctorName>(u_names[d]) = _velocity_names[d];

        for (const auto d : make_range(dimension()))
        {
          params.set<NonlinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
        }
      }
      {
        const std::string bc_type = "INSFVSymmetryPressureBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _pressure_name;
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};

        getProblem().addFVBC(bc_type, _pressure_name + "_" + boundary_name, params);
      }
    }
  }
}

void
WCNSFVFlowPhysics::addUserObjects()
{
  // Rhie Chow user object for interpolation velocities
  addRhieChowUserObjects();
}

void
WCNSFVFlowPhysics::addCorrectors()
{
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

bool
WCNSFVFlowPhysics::hasForchheimerFriction() const
{
  for (const auto block_i : index_range(_friction_types))
    for (const auto type_i : index_range(_friction_types[block_i]))
      if (MooseUtils::toUpper(_friction_types[block_i][type_i]) == "FORCHHEIMER")
        return true;
  return false;
}

unsigned short
WCNSFVFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  auto ghost_layers = WCNSFVFlowPhysicsBase::getNumberAlgebraicGhostingLayersNeeded();
  if (_porous_medium_treatment && isParamValid("porosity_smoothing_layers"))
    ghost_layers = std::max(getParam<unsigned short>("porosity_smoothing_layers"), ghost_layers);
  if ((_porous_medium_treatment &&
       getParam<MooseEnum>("porosity_interface_pressure_treatment") != "automatic") ||
      getParam<MooseEnum>("momentum_face_interpolation") == "skewness-corrected" ||
      getParam<MooseEnum>("pressure_face_interpolation") == "skewness-corrected")
    ghost_layers = std::max(ghost_layers, (unsigned short)3);
  return ghost_layers;
}

void
WCNSFVFlowPhysics::addRhieChowUserObjects()
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
    if (dynamic_cast<INSFVRhieChowInterpolator *>(obj))
    {
      const auto rc_obj = dynamic_cast<INSFVRhieChowInterpolator *>(obj);
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
      _porous_medium_treatment ? "PINSFVRhieChowInterpolator" : "INSFVRhieChowInterpolator";

  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    unsigned short smoothing_layers = isParamValid("porosity_smoothing_layers")
                                          ? getParam<unsigned short>("porosity_smoothing_layers")
                                          : 0;
    params.set<unsigned short>("smoothing_layers") = smoothing_layers;
  }

  if (!_has_flow_equations)
  {
    checkRhieChowFunctorsDefined();
    params.set<MooseFunctorName>("a_u") = "ax";
    params.set<MooseFunctorName>("a_v") = "ay";
    params.set<MooseFunctorName>("a_w") = "az";
  }

  params.applySpecificParameters(parameters(), INSFVRhieChowInterpolator::listOfCommonParams());
  getProblem().addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSFVFlowPhysics::checkRhieChowFunctorsDefined() const
{
  if (!getProblem().hasFunctor("ax", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ax must be provided for advection by auxiliary velocities");
  if (dimension() >= 2 && !getProblem().hasFunctor("ay", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient ay must be provided for advection by auxiliary velocities");
  if (dimension() == 3 && !getProblem().hasFunctor("az", /*thread_id=*/0))
    mooseError("Rhie Chow coefficient az must be provided for advection by auxiliary velocities");
}

UserObjectName
WCNSFVFlowPhysics::rhieChowUOName() const
{
  return (_porous_medium_treatment ? +"pins_rhie_chow_interpolator" : "ins_rhie_chow_interpolator");
}

MooseFunctorName
WCNSFVFlowPhysics::getLinearFrictionCoefName() const
{
  // Check all blocks. If more than one block, they would need to be consolidated #include in
  // a single functor material. We won't implement this for now
  if (_friction_types.empty())
    return "";
  else if (_friction_types.size() == 1)
  {
    for (const auto & type_i : index_range(_friction_types[0]))
    {
      const auto upper_name = MooseUtils::toUpper(_friction_types[0][type_i]);
      if (upper_name == "DARCY")
        return _friction_coeffs[0][type_i];
    }
    // No linear type found
    return "";
  }
  else if (_friction_types.size() > 1)
  {
    bool linear_friction_factor_found = false;
    MooseFunctorName linear_friction_factor;
    for (const auto block_i : index_range(_friction_types))
      for (const auto type_i : index_range(_friction_types[block_i]))
      {
        const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
        if (upper_name == "DARCY" && !linear_friction_factor_found)
        {
          linear_friction_factor_found = true;
          linear_friction_factor = _friction_types[block_i][type_i];
        }
        else if (upper_name == "DARCY" && !linear_friction_factor_found)
          if (linear_friction_factor != _friction_types[block_i][type_i])
            mooseError("Multiple linear friction factor with different names have been specified. "
                       "This is not currently supported as a single name should be retrievable. "
                       "Use a PiecewiseByBlockFunctorMaterial to consolidate them.");
      }
    if (linear_friction_factor_found)
      return linear_friction_factor;
    else
      return "";
  }
  mooseError("Should not get here");
}
