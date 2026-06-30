//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVConservativeSharpInterfaceVOFPhysics.h"

#include "NS.h"
#include "WCNSFVFlowPhysicsBase.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp",
                                     WCNSLinearFVConservativeSharpInterfaceVOFPhysics);
registerWCNSFVScalarTransportBaseTasks("NavierStokesApp",
                                       WCNSLinearFVConservativeSharpInterfaceVOFPhysics);
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeSharpInterfaceVOFPhysics,
                    "add_material");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeSharpInterfaceVOFPhysics,
                    "add_user_object");

InputParameters
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::validParams()
{
  InputParameters params = WCNSLinearFVScalarTransportPhysics::validParams();
  params.addClassDescription(
      "Create a linear-FV sharp-interface volume-fraction transport equation with an explicit "
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

  MooseEnum volume_fraction_outlet_type("inlet-outlet", "inlet-outlet");
  params.addParam<MooseEnum>(
      "volume_fraction_outlet_type",
      volume_fraction_outlet_type,
      "Outlet treatment for the transported volume fraction. The supported sharp-interface path "
      "imposes a backflow value on inflow and zero-gradient / extrapolation on outflow.");
  params.addParam<MooseFunctorName>(
      "volume_fraction_outlet_backflow_functor",
      "0",
      "Backflow value imposed by the inlet-outlet volume-fraction BC, typically 0 for air on "
      "an atmosphere patch.");
  params.addParam<bool>("volume_fraction_two_term_bc_expansion",
                        true,
                        "Whether the outlet outflow BC should use a two-term expansion.");

  params.addParam<MooseFunctorName>(
      "compression_factor",
      "0",
      "Compression coefficient c_alpha for the explicit interface-compression flux.");
  params.addParam<MooseFunctorName>(
      "interface_normal_functor",
      "interface_unit_normal_face",
      "Face-oriented interface unit normal used by the explicit compression flux.");
  params.addParam<std::vector<VariableName>>(
      "confined_scalar_variables",
      {},
      "Scalar amount variables q = alpha c to transport conservatively with the limited "
      "volume-fraction flux.");
  params.addParam<MooseFunctorName>(
      "confined_scalar_backflow_concentration",
      "0",
      "Concentration imposed for confined scalar backflow on open volume-fraction boundaries.");
  params.addRangeCheckedParam<Real>(
      "confined_scalar_alpha_floor",
      1e-12,
      "confined_scalar_alpha_floor > 0",
      "Minimum alpha used to recover confined scalar concentration c = q / alpha.");
  params.addParam<Real>(
      "confined_scalar_concentration_min", 0.0, "Minimum allowed confined scalar concentration.");
  params.addParam<Real>(
      "confined_scalar_concentration_max", 1.0, "Maximum allowed confined scalar concentration.");
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

  params.addParamNamesToGroup("system_names passive_scalar_names initial_scalar_variables "
                              "passive_scalar_advection_interpolation compression_factor "
                              "interface_normal_functor confined_scalar_variables "
                              "confined_scalar_backflow_concentration confined_scalar_alpha_floor "
                              "confined_scalar_concentration_min "
                              "confined_scalar_concentration_max",
                              "Numerical scheme");

  params.suppressParameter<MooseEnum>("preconditioning");
  return params;
}

WCNSLinearFVConservativeSharpInterfaceVOFPhysics::WCNSLinearFVConservativeSharpInterfaceVOFPhysics(
    const InputParameters & parameters)
  : WCNSLinearFVScalarTransportPhysics(parameters)
{
  if (_passive_scalar_names.size() != 1)
    paramError("passive_scalar_names",
               "The sharp-interface VOF physics supports exactly one transported volume-fraction "
               "variable.");

  if (_passive_scalar_names[0] == getParam<MooseFunctorName>("complementary_volume_fraction_name"))
    paramError("complementary_volume_fraction_name",
               "The complementary volume-fraction name must differ from the transported "
               "volume-fraction variable name.");

  if (getParam<MooseEnum>("passive_scalar_advection_interpolation") != "upwind")
    paramError("passive_scalar_advection_interpolation",
               "The sharp-interface VOF physics uses a bounded-base-plus-limited-correction "
               "structure, so the matrix transport must remain on donor/upwind transport.");
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addInitialConditions()
{
  if (!_define_variables && parameters().isParamSetByUser("initial_scalar_variables"))
    paramError(
        "initial_scalar_variables",
        "Volume-fraction variable is defined externally of "
        "WCNSLinearFVConservativeSharpInterfaceVOFPhysics, so should its initial condition.");

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
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addMaterials()
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
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addMixtureFunctorMaterial(
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
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addUserObjects()
{
  const std::string object_type = "ConservativeSharpInterfaceVOFMULESCorrector";
  const std::string object_name = prefix() + "vof_mules";
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  const auto & alpha_name = _passive_scalar_names[0];
  params.set<SolverSystemName>("system_name") = getSolverSystem(alpha_name);
  params.set<VariableName>("variable") = alpha_name;
  params.set<MooseFunctorName>("face_flux") = "vof_transport_phi";
  params.set<MooseFunctorName>("compression_factor") =
      getParam<MooseFunctorName>("compression_factor");
  params.set<MooseFunctorName>("interface_normal") =
      getParam<MooseFunctorName>("interface_normal_functor");
  params.set<unsigned int>("n_alpha_corrections") = getParam<unsigned int>("n_alpha_corrections");
  params.set<unsigned int>("n_limiter_iterations") = getParam<unsigned int>("n_limiter_iterations");
  params.set<MooseFunctorName>("liquid_density") =
      getParam<MooseFunctorName>("liquid_density_name");
  params.set<MooseFunctorName>("gas_density") = getParam<MooseFunctorName>("gas_density_name");
  params.set<std::vector<VariableName>>("confined_scalar_variables") =
      getParam<std::vector<VariableName>>("confined_scalar_variables");
  params.set<MooseFunctorName>("confined_scalar_backflow_concentration") =
      getParam<MooseFunctorName>("confined_scalar_backflow_concentration");
  params.set<Real>("confined_scalar_alpha_floor") = getParam<Real>("confined_scalar_alpha_floor");
  params.set<Real>("confined_scalar_concentration_min") =
      getParam<Real>("confined_scalar_concentration_min");
  params.set<Real>("confined_scalar_concentration_max") =
      getParam<Real>("confined_scalar_concentration_max");
  getProblem().addUserObject(object_type, object_name, params);
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addScalarAdvectionKernels()
{
  addLinearFVScalarAdvectionKernel(
      _passive_scalar_names[0],
      prefix() + "alpha_advection",
      _flow_equations_physics->rhieChowUOName(),
      getParam<MooseEnum>("passive_scalar_advection_interpolation"),
      _blocks,
      [](InputParameters & params)
      { params.set<MooseFunctorName>("face_flux") = "vof_transport_phi"; });
}

void
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addScalarOutletBC()
{
  const auto & outlet_boundaries = _flow_equations_physics->getOutletBoundaries();
  if (outlet_boundaries.empty())
    return;

  const auto & alpha_name = _passive_scalar_names[0];
  for (const auto & outlet_bdy : outlet_boundaries)
  {
    const std::string bc_type = "LinearFVInletOutletScalarBC";
    auto params = getFactory().getValidParams(bc_type);
    params.set<LinearVariableName>("variable") = alpha_name;
    params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
    params.set<bool>("use_two_term_expansion") =
        getParam<bool>("volume_fraction_two_term_bc_expansion");
    params.set<MooseFunctorName>("backflow_value") =
        getParam<MooseFunctorName>("volume_fraction_outlet_backflow_functor");
    params.set<MooseFunctorName>("face_flux") = "vof_transport_phi";
    getProblem().addLinearFVBC(bc_type, prefix() + "alpha_outlet_" + outlet_bdy, params);
  }
}
