//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.h"

#include "NS.h"
#include "WCNSFVFlowPhysicsBase.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp",
                                     WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics);
registerWCNSFVScalarTransportBaseTasks("NavierStokesApp",
                                       WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics);
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics,
                    "add_interpolation_method_physics");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics,
                    "add_material");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics,
                    "add_user_object");

InputParameters
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::validParams()
{
  InputParameters params = WCNSLinearFVScalarTransportPhysics::validParams();
  params.addClassDescription(
      "Create a linear-FV diffuse-interface volume-fraction transport equation with an explicit "
      "compression term and optional mixture-property functors.");

  params.set<std::vector<SolverSystemName>>("system_names") = {"alpha_system"};
  params.set<std::vector<NonlinearVariableName>>("passive_scalar_names") = {"alpha"};
  params.set<std::vector<FunctionName>>("initial_scalar_variables") = {"0"};
  params.set<MooseEnum>("passive_scalar_advection_interpolation") = "upwind";
  params.setDocString("passive_scalar_names",
                      "Singleton volume-fraction variable transported by the VOF equation.");
  params.setDocString("initial_scalar_variables",
                      "Initial condition for the transported volume fraction.");
  params.setDocString("passive_scalar_advection_interpolation",
                      "Matrix interpolation method for the volume-fraction transport equation.");
  params.addParam<MooseFunctorName>(
      "complementary_volume_fraction_name",
      "1_minus_alpha",
      "Name of the complementary gas/second-phase volume-fraction functor.");

  params.addParam<MooseFunctorName>(
      "volume_fraction_outlet_backflow_functor",
      "0",
      "Backflow value imposed by the inlet-outlet volume-fraction BC, typically 0 for air on "
      "an atmosphere patch.");
  params.addParam<bool>(
      "volume_fraction_two_term_bc_expansion",
      true,
      "Whether the volume fraction leaving through an outlet is extrapolated from the cell with "
      "its gradient. The extrapolation is applied as a limited MULES correction; the outlet "
      "boundary condition itself carries the cell value.");

  params.addParam<MooseFunctorName>(
      "compression_factor",
      "0",
      "Compression coefficient c_alpha for the explicit interface-compression flux.");
  params.addParam<MooseFunctorName>(
      "interface_normal_functor",
      "interface_unit_normal_face",
      "Face-oriented interface unit normal used by the explicit compression flux.");
  params.addRangeCheckedParam<unsigned int>("n_alpha_corrections",
                                            2,
                                            "n_alpha_corrections>=0",
                                            "Number of bounded explicit correction sweeps. Zero "
                                            "publishes the donor alpha flux without an "
                                            "explicit correction sweep.");
  params.addRangeCheckedParam<unsigned int>(
      "n_limiter_iterations",
      3,
      "n_limiter_iterations>0",
      "Number of limiter tightening passes inside each correction sweep.");
  params.addParam<MooseFunctorName>(
      "mixture_density_name", "rho_mixture", "Name of the generated mixture density functor.");
  params.addParam<MooseFunctorName>("mixture_dynamic_viscosity_name",
                                    "mu_mixture",
                                    "Name of the generated mixture dynamic viscosity functor.");
  params.addRequiredParam<MooseFunctorName>("liquid_density_name", "Liquid-phase density functor.");
  params.addRequiredParam<MooseFunctorName>("gas_density_name", "Gas-phase density functor.");
  params.addRequiredParam<MooseFunctorName>("liquid_dynamic_viscosity_name",
                                            "Liquid-phase dynamic viscosity functor.");
  params.addRequiredParam<MooseFunctorName>("gas_dynamic_viscosity_name",
                                            "Gas-phase dynamic viscosity functor.");

  params.addParamNamesToGroup("compression_factor interface_normal_functor", "Numerical scheme");

  params.suppressParameter<MooseEnum>("preconditioning");
  // The MULES correction relies on the upwind solve the constructor enforces; an externally
  // defined method would bypass that check
  params.suppressParameter<InterpolationMethodName>(
      "passive_scalar_advection_interpolation_method_name");
  return params;
}

WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::
    WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics(const InputParameters & parameters)
  : WCNSLinearFVScalarTransportPhysics(parameters)
{
  if (_passive_scalar_names.size() != 1)
    paramError("passive_scalar_names",
               "The diffuse-interface VOF physics supports exactly one transported volume-fraction "
               "variable.");

  if (_passive_scalar_names[0] == getParam<MooseFunctorName>("complementary_volume_fraction_name"))
    paramError("complementary_volume_fraction_name",
               "The complementary volume-fraction name must differ from the transported "
               "volume-fraction variable name.");

  // MULES relies on the implicit solve being bounded, which only the upwind flux guarantees; the
  // higher-order face value enters through the corrector's explicit, limited correction instead
  if (getParam<MooseEnum>("passive_scalar_advection_interpolation") != "upwind")
    paramError("passive_scalar_advection_interpolation",
               "The diffuse-interface VOF physics uses a bounded-base-plus-limited-correction "
               "structure, so the matrix transport must remain on donor/upwind transport.");
}

void
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addInitialConditions()
{
  if (!_define_variables && parameters().isParamSetByUser("initial_scalar_variables"))
    paramError(
        "initial_scalar_variables",
        "Volume-fraction variable is defined externally of "
        "WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics, so should its initial condition.");

  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  const auto & alpha_name = _passive_scalar_names[0];
  if (!shouldCreateIC(alpha_name,
                      _blocks,
                      /*whether IC is a default*/ !isParamSetByUser("initial_scalar_variables"),
                      /*error if already an IC*/ isParamSetByUser("initial_scalar_variables")))
    return;

  auto params = getFactory().getValidParams("FVFunctionIC");
  assignBlocks(params, _blocks);
  params.set<VariableName>("variable") = alpha_name;
  params.set<FunctionName>("function") =
      getParam<std::vector<FunctionName>>("initial_scalar_variables")[0];
  getProblem().addFVInitialCondition("FVFunctionIC", prefix() + alpha_name + "_ic", params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addMaterials()
{
  const auto gas_fraction_name = getParam<MooseFunctorName>("complementary_volume_fraction_name");
  if (!getProblem().hasFunctor(gas_fraction_name, /*tid=*/0))
  {
    const auto & alpha_name = _passive_scalar_names[0];
    auto params = getFactory().getValidParams("ParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::string>("expression") = "1 - " + alpha_name;
    params.set<std::vector<std::string>>("functor_names") = {alpha_name};
    params.set<std::string>("property_name") = gas_fraction_name;
    params.set<std::vector<std::string>>("output_properties") = {gas_fraction_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial("ParsedFunctorMaterial", prefix() + "complementary_fraction", params);
  }

  // The density is not clamped: the momentum time derivative must see exactly the density the
  // transported alpha implies, or the mass it conserves differs from the mass rho_phi moved. The
  // viscosity is clamped, since a slight overshoot of alpha there only changes a coefficient.
  addMixtureFunctorMaterial("mixture_density",
                            getParam<MooseFunctorName>("mixture_density_name"),
                            getParam<MooseFunctorName>("liquid_density_name"),
                            getParam<MooseFunctorName>("gas_density_name"),
                            false);
  addMixtureFunctorMaterial("mixture_dynamic_viscosity",
                            getParam<MooseFunctorName>("mixture_dynamic_viscosity_name"),
                            getParam<MooseFunctorName>("liquid_dynamic_viscosity_name"),
                            getParam<MooseFunctorName>("gas_dynamic_viscosity_name"),
                            true);
}

void
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addMixtureFunctorMaterial(
    const std::string & object_suffix,
    const MooseFunctorName & property_name,
    const MooseFunctorName & phase_1_name,
    const MooseFunctorName & phase_2_name,
    const bool limit_phase_fraction)
{
  auto params = getFactory().getValidParams("WCNSLinearFVMixtureFunctorMaterial");
  assignBlocks(params, _blocks);
  params.set<std::vector<MooseFunctorName>>("prop_names") = {property_name};
  params.set<std::vector<MooseFunctorName>>("phase_1_names") = {phase_1_name};
  params.set<std::vector<MooseFunctorName>>("phase_2_names") = {phase_2_name};
  params.set<MooseFunctorName>("phase_1_fraction") = _passive_scalar_names[0];
  params.set<bool>("limit_phase_fraction") = limit_phase_fraction;
  getProblem().addMaterial("WCNSLinearFVMixtureFunctorMaterial", prefix() + object_suffix, params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addUserObjects()
{
  // The corrector transports with the same frozen flux as the advection kernel, so the limited
  // flux it publishes is the correction of exactly the donor flux the matrix solve used
  const std::string object_type = "ConservativeDiffuseInterfaceVOFMULESCorrector";
  const std::string object_name = prefix() + "vof_mules";
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  const auto & alpha_name = _passive_scalar_names[0];
  params.set<SolverSystemName>("system_name") = getSolverSystem(alpha_name);
  params.set<VariableName>("variable") = alpha_name;
  params.set<MooseFunctorName>("face_flux") = "lagged_corrected_face_phi";
  params.set<MooseFunctorName>("compression_factor") =
      getParam<MooseFunctorName>("compression_factor");
  params.set<MooseFunctorName>("interface_normal") =
      getParam<MooseFunctorName>("interface_normal_functor");
  params.set<unsigned int>("n_alpha_corrections") = getParam<unsigned int>("n_alpha_corrections");
  params.set<unsigned int>("n_limiter_iterations") = getParam<unsigned int>("n_limiter_iterations");
  params.set<bool>("outflow_two_term_expansion") =
      getParam<bool>("volume_fraction_two_term_bc_expansion");
  params.set<MooseFunctorName>("liquid_density") =
      getParam<MooseFunctorName>("liquid_density_name");
  params.set<MooseFunctorName>("gas_density") = getParam<MooseFunctorName>("gas_density_name");
  getProblem().addUserObject(object_type, object_name, params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addScalarAdvectionKernels()
{
  const InterpolationMethodName method_name(
      std::string(getParam<MooseEnum>("passive_scalar_advection_interpolation")));

  const std::string kernel_type = "LinearFVScalarAdvection";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<LinearVariableName>("variable") = _passive_scalar_names[0];
  params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
  params.set<InterpolationMethodName>("advected_interp_method_name") = method_name;
  // The volume fraction advances with the flux frozen for this outer iteration, so that the
  // limited flux it publishes and the backflow switches that read it agree.
  params.set<MooseFunctorName>("face_flux") = "lagged_corrected_face_phi";
  getProblem().addLinearFVKernel(kernel_type, prefix() + "alpha_advection", params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addScalarOutletBC()
{
  const auto & outlet_boundaries = _flow_equations_physics->getOutletBoundaries();
  if (outlet_boundaries.empty())
    return;

  // An outlet lets alpha leave with the flow and brings the backflow value in where the flux
  // reverses; the switch reads the same frozen flux as the transport, so a face cannot be an
  // outflow for the kernel and an inflow for its boundary condition within one outer iteration
  const auto & alpha_name = _passive_scalar_names[0];
  for (const auto & outlet_bdy : outlet_boundaries)
  {
    const std::string bc_type = "LinearFVInletOutletScalarBC";
    auto params = getFactory().getValidParams(bc_type);
    params.set<LinearVariableName>("variable") = alpha_name;
    params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
    // The upwind solve carries the cell value out; the corrector adds the two-term extrapolation
    // as a limited correction when volume_fraction_two_term_bc_expansion is set
    params.set<bool>("use_two_term_expansion") = false;
    params.set<MooseFunctorName>("backflow_value") =
        getParam<MooseFunctorName>("volume_fraction_outlet_backflow_functor");
    params.set<MooseFunctorName>("face_flux") = "lagged_corrected_face_phi";
    getProblem().addLinearFVBC(bc_type, prefix() + "alpha_outlet_" + outlet_bdy, params);
  }
}
