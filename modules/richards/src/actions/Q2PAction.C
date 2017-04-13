/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Q2PAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<Q2PAction>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addRequiredParam<NonlinearVariableName>("porepressure", "The porepressure variable");
  params.addRequiredParam<NonlinearVariableName>("saturation", "The water saturation variable");
  params.addRequiredParam<UserObjectName>(
      "water_density",
      "A RichardsDensity UserObject that defines the water density as a function of porepressure.");
  params.addRequiredParam<UserObjectName>(
      "water_relperm",
      "A RichardsRelPerm UserObject that defines the water relative permeability "
      "as a function of water saturation (eg RichardsRelPermPower).");
  params.addParam<UserObjectName>(
      "water_relperm_for_diffusion",
      "A RichardsRelPerm UserObject that defines the water relative permeability as a function of "
      "water saturation that will be used in the diffusivity Kernel (eg RichardsRelPermPower).  If "
      "not given, water_relperm will be used instead, which is the most common use-case.");
  params.addRequiredParam<Real>("water_viscosity", "The water viscosity");
  params.addRequiredParam<UserObjectName>(
      "gas_density",
      "A RichardsDensity UserObject that defines the gas density as a function of porepressure.");
  params.addRequiredParam<UserObjectName>(
      "gas_relperm",
      "A RichardsRelPerm UserObject that defines the gas relative permeability as a "
      "function of water saturation (eg Q2PRelPermPowerGas).");
  params.addRequiredParam<Real>("gas_viscosity", "The gas viscosity");
  params.addRequiredParam<Real>("diffusivity", "The diffusivity");
  params.addParam<std::vector<OutputName>>("output_nodal_masses_to",
                                           "Output Nodal masses to this Output object.  If you "
                                           "don't want any outputs, don't input anything here");
  params.addParam<std::vector<OutputName>>(
      "output_total_masses_to",
      "Output total water and gas mass to this Output object.  If you "
      "don't want any outputs, don't input anything here");
  params.addParam<bool>("save_gas_flux_in_Q2PGasFluxResidual",
                        false,
                        "Save the residual for the "
                        "Q2PPorepressureFlux into "
                        "the AuxVariable called "
                        "Q2PGasFluxResidual");
  params.addParam<bool>("save_water_flux_in_Q2PWaterFluxResidual",
                        false,
                        "Save the residual for the Q2PSaturationFlux into the AuxVariable called "
                        "Q2PWaterFluxResidual");
  params.addParam<bool>("save_gas_Jacobian_in_Q2PGasJacobian",
                        false,
                        "Save the diagonal component of the Q2PPorepressureFlux Jacobian into the "
                        "AuxVariable called Q2PGasJacobian");
  params.addParam<bool>("save_water_Jacobian_in_Q2PWaterJacobian",
                        false,
                        "Save the diagonal component of the Q2PSaturationFlux Jacobian into the "
                        "AuxVariable called Q2PWaterJacobian");
  params.addParam<MooseEnum>("ORDER",
                             orders,
                             "The order for the porepressure and saturation: " +
                                 orders.getRawNames() +
                                 " (only needed if you're calculating masses)");
  return params;
}

Q2PAction::Q2PAction(const InputParameters & params)
  : Action(params),
    _pp_var(getParam<NonlinearVariableName>("porepressure")),
    _sat_var(getParam<NonlinearVariableName>("saturation")),
    _water_density(getParam<UserObjectName>("water_density")),
    _water_relperm(getParam<UserObjectName>("water_relperm")),
    _water_relperm_for_diffusivity(isParamValid("water_relperm_for_diffusivity")
                                       ? getParam<UserObjectName>("water_relperm_for_diffusivity")
                                       : getParam<UserObjectName>("water_relperm")),
    _water_viscosity(getParam<Real>("water_viscosity")),
    _gas_density(getParam<UserObjectName>("gas_density")),
    _gas_relperm(getParam<UserObjectName>("gas_relperm")),
    _gas_viscosity(getParam<Real>("gas_viscosity")),
    _diffusivity(getParam<Real>("diffusivity")),
    _output_nodal_masses_to(getParam<std::vector<OutputName>>("output_nodal_masses_to")),
    _output_total_masses_to(getParam<std::vector<OutputName>>("output_total_masses_to")),
    _save_gas_flux_in_Q2PGasFluxResidual(getParam<bool>("save_gas_flux_in_Q2PGasFluxResidual")),
    _save_water_flux_in_Q2PWaterFluxResidual(
        getParam<bool>("save_water_flux_in_Q2PWaterFluxResidual")),
    _save_gas_Jacobian_in_Q2PGasJacobian(getParam<bool>("save_gas_Jacobian_in_Q2PGasJacobian")),
    _save_water_Jacobian_in_Q2PWaterJacobian(
        getParam<bool>("save_water_Jacobian_in_Q2PWaterJacobian"))
{
  _nodal_masses_not_outputted = false;
  if (_output_nodal_masses_to.size() == 0)
    _nodal_masses_not_outputted = true;

  _total_masses_not_outputted = false;
  if (_output_total_masses_to.size() == 0)
    _total_masses_not_outputted = true;

  _no_mass_calculations = (_nodal_masses_not_outputted && _total_masses_not_outputted);
}

void
Q2PAction::act()
{
  // add the kernels
  if (_current_task == "add_kernel")
  {
    std::string kernel_name;
    std::string kernel_type;
    InputParameters params = _factory.getValidParams("Q2PNodalMass");

    kernel_name = "Q2P_nodal_water_mass";
    kernel_type = "Q2PNodalMass";
    params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _sat_var;
    params.set<std::vector<VariableName>>("other_var") = {_pp_var};
    params.set<bool>("var_is_porepressure") = false;
    if (!_no_mass_calculations)
      params.set<std::vector<AuxVariableName>>("save_in") = {"Q2P_nodal_water_mass_divided_by_dt"};
    params.set<UserObjectName>("fluid_density") = _water_density;
    _problem->addKernel(kernel_type, kernel_name, params);

    kernel_name = "Q2P_nodal_gas_mass";
    kernel_type = "Q2PNodalMass";
    params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _pp_var;
    params.set<std::vector<VariableName>>("other_var") = {_sat_var};
    params.set<bool>("var_is_porepressure") = true;
    if (!_no_mass_calculations)
      params.set<std::vector<AuxVariableName>>("save_in") = {"Q2P_nodal_gas_mass_divided_by_dt"};
    params.set<UserObjectName>("fluid_density") = _gas_density;
    _problem->addKernel(kernel_type, kernel_name, params);

    kernel_name = "Q2P_nodal_water_mass_old";
    kernel_type = "Q2PNegativeNodalMassOld";
    params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _sat_var;
    params.set<std::vector<VariableName>>("other_var") = {_pp_var};
    params.set<bool>("var_is_porepressure") = false;
    params.set<UserObjectName>("fluid_density") = _water_density;
    _problem->addKernel(kernel_type, kernel_name, params);

    kernel_name = "Q2P_nodal_gas_mass_old";
    kernel_type = "Q2PNegativeNodalMassOld";
    params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _pp_var;
    params.set<std::vector<VariableName>>("other_var") = {_sat_var};
    params.set<bool>("var_is_porepressure") = true;
    params.set<UserObjectName>("fluid_density") = _gas_density;
    _problem->addKernel(kernel_type, kernel_name, params);

    kernel_name = "Q2P_water_flux";
    kernel_type = "Q2PSaturationFlux";
    params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _sat_var;
    params.set<std::vector<VariableName>>("porepressure_variable") = {_pp_var};
    params.set<UserObjectName>("fluid_density") = _water_density;
    params.set<UserObjectName>("fluid_relperm") = _water_relperm;
    params.set<Real>("fluid_viscosity") = _water_viscosity;
    if (_save_water_flux_in_Q2PWaterFluxResidual)
      params.set<std::vector<AuxVariableName>>("save_in") = {"Q2PWaterFluxResidual"};
    if (_save_water_Jacobian_in_Q2PWaterJacobian)
      params.set<std::vector<AuxVariableName>>("diag_save_in") = {"Q2PWaterJacobian"};
    _problem->addKernel(kernel_type, kernel_name, params);

    kernel_name = "Q2P_gas_flux";
    kernel_type = "Q2PPorepressureFlux";
    params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _pp_var;
    params.set<std::vector<VariableName>>("saturation_variable") = {_sat_var};
    params.set<UserObjectName>("fluid_density") = _gas_density;
    params.set<UserObjectName>("fluid_relperm") = _gas_relperm;
    params.set<Real>("fluid_viscosity") = _gas_viscosity;
    if (_save_gas_flux_in_Q2PGasFluxResidual)
      params.set<std::vector<AuxVariableName>>("save_in") = {"Q2PGasFluxResidual"};
    if (_save_gas_Jacobian_in_Q2PGasJacobian)
      params.set<std::vector<AuxVariableName>>("diag_save_in") = {"Q2PGasJacobian"};
    _problem->addKernel(kernel_type, kernel_name, params);

    kernel_name = "Q2P_liquid_diffusion";
    kernel_type = "Q2PSaturationDiffusion";
    params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _sat_var;
    params.set<std::vector<VariableName>>("porepressure_variable") = {_pp_var};
    params.set<UserObjectName>("fluid_density") = _water_density;
    params.set<UserObjectName>("fluid_relperm") = _water_relperm_for_diffusivity;
    params.set<Real>("fluid_viscosity") = _water_viscosity;
    params.set<Real>("diffusivity") = _diffusivity;
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if (_current_task == "add_aux_variable")
  {
    if (!_no_mass_calculations)
    {
      // user wants nodal masses or total masses
      _problem->addAuxVariable("Q2P_nodal_water_mass_divided_by_dt",
                               FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("ORDER")),
                                      Utility::string_to_enum<FEFamily>("LAGRANGE")));
      _problem->addAuxVariable("Q2P_nodal_gas_mass_divided_by_dt",
                               FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("ORDER")),
                                      Utility::string_to_enum<FEFamily>("LAGRANGE")));
    }
    if (_save_gas_flux_in_Q2PGasFluxResidual)
      _problem->addAuxVariable("Q2PGasFluxResidual",
                               FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("ORDER")),
                                      Utility::string_to_enum<FEFamily>("LAGRANGE")));
    if (_save_water_flux_in_Q2PWaterFluxResidual)
      _problem->addAuxVariable("Q2PWaterFluxResidual",
                               FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("ORDER")),
                                      Utility::string_to_enum<FEFamily>("LAGRANGE")));
    if (_save_gas_Jacobian_in_Q2PGasJacobian)
      _problem->addAuxVariable("Q2PGasJacobian",
                               FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("ORDER")),
                                      Utility::string_to_enum<FEFamily>("LAGRANGE")));
    if (_save_water_Jacobian_in_Q2PWaterJacobian)
      _problem->addAuxVariable("Q2PWaterJacobian",
                               FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("ORDER")),
                                      Utility::string_to_enum<FEFamily>("LAGRANGE")));
  }

  if (_current_task == "add_function" && _output_total_masses_to.size() > 0)
  {
    // user wants total masses, so need to build Functions to do this
    InputParameters params = _factory.getValidParams("ParsedFunction");

    params.set<std::string>("value") = "a*b";

    std::vector<std::string> vars;
    vars.push_back("a");
    vars.push_back("b");
    params.set<std::vector<std::string>>("vars") = vars;

    std::vector<std::string> vals_water;
    vals_water.push_back("Q2P_mass_water_divided_by_dt");
    vals_water.push_back("Q2P_dt");
    params.set<std::vector<std::string>>("vals") = vals_water;
    _problem->addFunction("ParsedFunction", "Q2P_water_mass_fcn", params);

    std::vector<std::string> vals_gas;
    vals_gas.push_back("Q2P_mass_gas_divided_by_dt");
    vals_gas.push_back("Q2P_dt");
    params.set<std::vector<std::string>>("vals") = vals_gas;
    _problem->addFunction("ParsedFunction", "Q2P_gas_mass_fcn", params);
  }

  if (_current_task == "add_postprocessor" && _output_total_masses_to.size() > 0)
  {
    // user wants total masses, so need to build Postprocessors to do this

    InputParameters params = _factory.getValidParams("TimestepSize");

    MooseUtils::setExecuteOnFlags(params, 1, EXEC_TIMESTEP_BEGIN);
    params.set<std::vector<OutputName>>("outputs") = {"none"};
    _problem->addPostprocessor("TimestepSize", "Q2P_dt", params);

    params = _factory.getValidParams("NodalSum");
    params.set<std::vector<OutputName>>("outputs") = {"none"};
    params.set<std::vector<VariableName>>("variable") = {"Q2P_nodal_water_mass_divided_by_dt"};
    _problem->addPostprocessor("NodalSum", "Q2P_mass_water_divided_by_dt", params);

    params = _factory.getValidParams("FunctionValuePostprocessor");
    params.set<FunctionName>("function") = "Q2P_water_mass_fcn";
    params.set<std::vector<OutputName>>("outputs") = _output_total_masses_to;
    _problem->addPostprocessor("FunctionValuePostprocessor", "mass_water", params);

    params = _factory.getValidParams("NodalSum");
    params.set<std::vector<OutputName>>("outputs") = {"none"};
    params.set<std::vector<VariableName>>("variable") = {"Q2P_nodal_gas_mass_divided_by_dt"};
    _problem->addPostprocessor("NodalSum", "Q2P_mass_gas_divided_by_dt", params);

    params = _factory.getValidParams("FunctionValuePostprocessor");
    params.set<FunctionName>("function") = "Q2P_gas_mass_fcn";
    params.set<std::vector<OutputName>>("outputs") = _output_total_masses_to;
    _problem->addPostprocessor("FunctionValuePostprocessor", "mass_gas", params);
  }
}
