//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSAction.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseUtils.h"
#include "NestedSolve.h"

#include "libmesh/int_range.h"
#include "libmesh/string_to_enum.h"

#include <set>

using namespace libMesh;

registerMooseAction("PhaseFieldApp", KKSAction, "add_variable");
registerMooseAction("PhaseFieldApp", KKSAction, "add_material");
registerMooseAction("PhaseFieldApp", KKSAction, "add_kernel");

InputParameters
KKSAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set up the variables, kernels, and optional nested-solve materials for a multiphase, "
      "multicomponent KKS model using the split Cahn-Hilliard formulation.");

  MooseEnum phase_concentration_solve("GLOBAL NESTED");
  params.addRequiredParam<MooseEnum>("phase_concentration_solve",
                                     phase_concentration_solve,
                                     "Whether phase concentrations are nonlinear variables in the "
                                     "global system or material properties from a nested solve");
  MooseEnum phase_constraint("NONE LAGRANGE", "LAGRANGE");
  params.addParam<MooseEnum>(
      "phase_constraint",
      phase_constraint,
      "Method used to constrain the phase switching functions; select NONE for switching "
      "functions that are normalized by construction");

  params.addRequiredParam<std::vector<std::string>>(
      "phase_names", "Short phase names used to generate phase concentration names");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "order_parameters", "One nonlinear order parameter name for each phase");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "global_concentrations", "Independent global concentration variable names");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "free_energies", "Phase free energy names in phase_names order");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "switching_functions", "Phase switching function names in phase_names order");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "barrier_functions", "Phase barrier function names in phase_names order");
  params.addRequiredParam<std::vector<Real>>("barrier_heights",
                                             "Barrier height for each phase in phase_names order");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "concentration_mobilities", "Mobility for each independent global concentration");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "order_parameter_mobilities", "Mobility for each phase order parameter");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "kappas", "Gradient energy coefficient for each phase order parameter");
  params.addParam<std::vector<AuxVariableName>>(
      "phase_concentration_initial_values",
      "Initial phase concentrations or auxiliary variables containing them for the nested solve, "
      "with phase varying fastest");

  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>(
      "family", families, "Finite element family for generated nonlinear variables");
  params.addParam<MooseEnum>(
      "order", orders, "Finite element order for generated nonlinear variables");
  params.addParam<Real>("scaling", 1.0, "Scaling applied to generated nonlinear variables");
  params.addParam<bool>("implicit", true, "Whether generated kernels are implicit");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether generated kernels use the displaced mesh");
  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "Block restriction for generated variables, kernels, and materials");
  params.addParam<NonlinearVariableName>(
      "lagrange_multiplier", "kks_lambda", "Name of the generated Lagrange multiplier variable");
  params.addParam<Real>(
      "constraint_epsilon", 1e-9, "Shift used to avoid a zero Lagrange multiplier pivot");

  params += NestedSolve::validParams();
  params.addParam<bool>("damped_Newton", false, "Use damped Newton for the nested solve");
  params.addParam<MaterialName>(
      "conditions", "C", "Material that checks nested-solve bounds when damping is enabled");

  params.addParamNamesToGroup("family order scaling implicit use_displaced_mesh block", "Advanced");
  params.addParamNamesToGroup(
      "phase_concentration_initial_values relative_tolerance absolute_tolerance "
      "step_size_tolerance min_iterations max_iterations acceptable_multiplier damping_factor "
      "max_damping_iterations damped_Newton conditions",
      "Nested solve");
  return params;
}

KKSAction::KKSAction(const InputParameters & params)
  : Action(params),
    _phase_concentration_solve(
        getParam<MooseEnum>("phase_concentration_solve").getEnum<PhaseConcentrationSolve>()),
    _phase_constraint(getParam<MooseEnum>("phase_constraint").getEnum<PhaseConstraint>()),
    _phase_names(getParam<std::vector<std::string>>("phase_names")),
    _order_parameters(getParam<std::vector<NonlinearVariableName>>("order_parameters")),
    _global_concentrations(getParam<std::vector<NonlinearVariableName>>("global_concentrations")),
    _free_energies(getParam<std::vector<MaterialPropertyName>>("free_energies")),
    _switching_functions(getParam<std::vector<MaterialPropertyName>>("switching_functions")),
    _barrier_functions(getParam<std::vector<MaterialPropertyName>>("barrier_functions")),
    _barrier_heights(getParam<std::vector<Real>>("barrier_heights")),
    _concentration_mobilities(
        getParam<std::vector<MaterialPropertyName>>("concentration_mobilities")),
    _order_parameter_mobilities(
        getParam<std::vector<MaterialPropertyName>>("order_parameter_mobilities")),
    _kappas(getParam<std::vector<MaterialPropertyName>>("kappas")),
    _fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
    _scaling(getParam<Real>("scaling"))
{
  const auto num_phases = _phase_names.size();
  const auto num_components = _global_concentrations.size();

  if (num_phases < 2)
    paramError("phase_names", "KKSAction requires at least two phases");
  if (num_components == 0)
    paramError("global_concentrations", "KKSAction requires at least one concentration");

  const auto require_phase_size = [this, num_phases](const auto & values, const auto & parameter)
  {
    if (values.size() != num_phases)
      paramError(parameter, "Expected one entry for each of the ", num_phases, " phases");
  };
  require_phase_size(_order_parameters, "order_parameters");
  require_phase_size(_free_energies, "free_energies");
  require_phase_size(_switching_functions, "switching_functions");
  require_phase_size(_barrier_functions, "barrier_functions");
  require_phase_size(_barrier_heights, "barrier_heights");
  require_phase_size(_order_parameter_mobilities, "order_parameter_mobilities");
  require_phase_size(_kappas, "kappas");

  if (_concentration_mobilities.size() != num_components)
    paramError("concentration_mobilities",
               "Expected one entry for each of the ",
               num_components,
               " independent concentrations");

  if (_phase_concentration_solve == PhaseConcentrationSolve::NESTED)
  {
    if (!isParamValid("phase_concentration_initial_values"))
      paramError("phase_concentration_initial_values",
                 "This parameter is required for a nested phase concentration solve");
    if (getParam<std::vector<AuxVariableName>>("phase_concentration_initial_values").size() !=
        num_phases * num_components)
      paramError("phase_concentration_initial_values",
                 "Expected ",
                 num_phases * num_components,
                 " entries with phase varying fastest");
  }

  std::set<std::string> phase_names(_phase_names.begin(), _phase_names.end());
  if (phase_names.size() != num_phases)
    paramError("phase_names", "Phase names must be unique");

  std::set<std::string> variable_names;
  for (const auto & name : _order_parameters)
    if (!variable_names.insert(name).second)
      paramError(
          "order_parameters", "Generated variable names must be unique; duplicate '", name, "'");
  for (const auto & name : _global_concentrations)
    if (!variable_names.insert(name).second)
      paramError("global_concentrations",
                 "Generated variable names must be unique; duplicate '",
                 name,
                 "'");
  for (const auto & concentration : _global_concentrations)
    if (!variable_names.insert("mu_" + concentration).second)
      paramError("global_concentrations", "Generated chemical potential name is not unique");

  if (_phase_concentration_solve == PhaseConcentrationSolve::GLOBAL)
    for (const auto & name : phaseConcentrationProperties())
      if (!variable_names.insert(name).second)
        paramError("phase_names", "Generated phase concentration name is not unique: '", name, "'");

  if (_phase_constraint == PhaseConstraint::LAGRANGE &&
      !variable_names.insert(getParam<NonlinearVariableName>("lagrange_multiplier")).second)
    paramError("lagrange_multiplier",
               "The Lagrange multiplier name conflicts with another variable");
}

void
KKSAction::act()
{
  if (_current_task == "add_variable")
    addVariables();
  else if (_current_task == "add_material")
    addMaterials();
  else if (_current_task == "add_kernel")
    addKernels();
  else
    mooseError("Internal error in KKSAction");
}

std::vector<VariableName>
KKSAction::phaseConcentrations(unsigned int component) const
{
  std::vector<VariableName> names;
  names.reserve(_phase_names.size());
  for (const auto & phase : _phase_names)
    names.emplace_back(_global_concentrations[component] + "_" + phase);
  return names;
}

std::vector<MaterialPropertyName>
KKSAction::phaseConcentrationProperties() const
{
  std::vector<MaterialPropertyName> names;
  names.reserve(_global_concentrations.size() * _phase_names.size());
  for (const auto component : index_range(_global_concentrations))
    for (const auto & name : phaseConcentrations(component))
      names.emplace_back(name);
  return names;
}

std::vector<VariableName>
KKSAction::otherOrderParameters(unsigned int phase) const
{
  std::vector<VariableName> names;
  names.reserve(_order_parameters.size() - 1);
  for (const auto other_phase : index_range(_order_parameters))
    if (other_phase != phase)
      names.emplace_back(_order_parameters[other_phase]);
  return names;
}

std::vector<VariableName>
KKSAction::phaseArguments(unsigned int phase, unsigned int excluded_component) const
{
  std::vector<VariableName> names;
  names.reserve(_global_concentrations.size() - 1);
  for (const auto component : index_range(_global_concentrations))
    if (component != excluded_component)
      names.emplace_back(phaseConcentrations(component)[phase]);
  return names;
}

std::vector<VariableName>
KKSAction::acCoupledVariables(unsigned int phase) const
{
  std::vector<VariableName> names;
  names.reserve(_global_concentrations.size() * (_phase_names.size() + 1) +
                _order_parameters.size() - 1);
  names.insert(names.end(), _global_concentrations.begin(), _global_concentrations.end());
  if (_phase_concentration_solve == PhaseConcentrationSolve::GLOBAL)
    for (const auto component : index_range(_global_concentrations))
    {
      const auto phase_concentrations = phaseConcentrations(component);
      names.insert(names.end(), phase_concentrations.begin(), phase_concentrations.end());
    }
  const auto other_etas = otherOrderParameters(phase);
  names.insert(names.end(), other_etas.begin(), other_etas.end());
  return names;
}

void
KKSAction::applyKernelParameters(InputParameters & params) const
{
  params.applySpecificParameters(parameters(), {"block", "implicit", "use_displaced_mesh"});
}

void
KKSAction::addVariables()
{
  const auto type = AddVariableAction::variableType(_fe_type);
  auto params = _factory.getValidParams(type);
  params.set<MooseEnum>("family") = Moose::stringify(_fe_type.family);
  params.set<MooseEnum>("order") = _fe_type.order.get_order();
  params.set<std::vector<Real>>("scaling") = {_scaling};
  params.applySpecificParameters(parameters(), {"block"});

  for (const auto & concentration : _global_concentrations)
  {
    _problem->addVariable(type, concentration, params);
    _problem->addVariable(type, "mu_" + concentration, params);
  }
  for (const auto & eta : _order_parameters)
    _problem->addVariable(type, eta, params);

  if (_phase_concentration_solve == PhaseConcentrationSolve::GLOBAL)
    for (const auto & concentration : phaseConcentrationProperties())
      _problem->addVariable(type, concentration, params);

  if (_phase_constraint == PhaseConstraint::LAGRANGE)
    _problem->addVariable(type, getParam<NonlinearVariableName>("lagrange_multiplier"), params);
}

void
KKSAction::addMaterials()
{
  if (_phase_concentration_solve != PhaseConcentrationSolve::NESTED)
    return;

  const auto ci_names = phaseConcentrationProperties();
  const std::vector<VariableName> global_cs(_global_concentrations.begin(),
                                            _global_concentrations.end());
  const std::vector<VariableName> all_etas(_order_parameters.begin(), _order_parameters.end());
  const std::vector<MaterialName> free_energy_materials(_free_energies.begin(),
                                                        _free_energies.end());

  auto params = _factory.getValidParams("KKSPhaseConcentrationMultiPhaseMaterial");
  params.set<std::vector<VariableName>>("global_cs") = global_cs;
  params.set<std::vector<VariableName>>("all_etas") = all_etas;
  params.set<std::vector<MaterialPropertyName>>("hj_names") = _switching_functions;
  params.set<std::vector<MaterialName>>("Fj_names") = free_energy_materials;
  params.set<std::vector<MaterialPropertyName>>("ci_names") = ci_names;
  const auto & initial_values =
      getParam<std::vector<AuxVariableName>>("phase_concentration_initial_values");
  // Action-created material parameters bypass the parser's coupled-default conversion.
  std::vector<Real> initial_defaults;
  initial_defaults.reserve(initial_values.size());
  for (const auto & initial_value : initial_values)
  {
    Real value;
    if (!MooseUtils::convert<Real>(initial_value, value, false))
    {
      initial_defaults.clear();
      break;
    }
    initial_defaults.emplace_back(value);
  }
  if (initial_defaults.empty())
    params.setCoupledVar("ci_IC",
                         std::vector<VariableName>(initial_values.begin(), initial_values.end()));
  else
  {
    params.setCoupledVar("ci_IC", std::vector<VariableName>{});
    for (const auto i : index_range(initial_defaults))
      params.defaultCoupledValue("ci_IC", initial_defaults[i], i);
  }
  params.applySpecificParameters(parameters(),
                                 {"block",
                                  "relative_tolerance",
                                  "absolute_tolerance",
                                  "step_size_tolerance",
                                  "min_iterations",
                                  "max_iterations",
                                  "acceptable_multiplier",
                                  "damping_factor",
                                  "max_damping_iterations",
                                  "damped_Newton",
                                  "conditions"});
  _problem->addMaterial(
      "KKSPhaseConcentrationMultiPhaseMaterial", "kks_phase_concentrations", params);

  params = _factory.getValidParams("KKSPhaseConcentrationMultiPhaseDerivatives");
  params.set<std::vector<VariableName>>("global_cs") = global_cs;
  params.set<std::vector<VariableName>>("all_etas") = all_etas;
  params.set<std::vector<MaterialName>>("Fj_names") = free_energy_materials;
  params.set<std::vector<MaterialPropertyName>>("hj_names") = _switching_functions;
  params.set<std::vector<MaterialPropertyName>>("ci_names") = ci_names;
  params.applySpecificParameters(parameters(), {"block"});
  _problem->addMaterial(
      "KKSPhaseConcentrationMultiPhaseDerivatives", "kks_phase_concentration_derivatives", params);
}

void
KKSAction::addKernels()
{
  const std::vector<VariableName> global_cs(_global_concentrations.begin(),
                                            _global_concentrations.end());
  const std::vector<VariableName> all_etas(_order_parameters.begin(), _order_parameters.end());
  const auto ci_names = phaseConcentrationProperties();

  for (const auto component : index_range(_global_concentrations))
  {
    const auto & concentration = _global_concentrations[component];
    const NonlinearVariableName chemical_potential = "mu_" + concentration;

    auto params = _factory.getValidParams("CoupledTimeDerivative");
    params.set<NonlinearVariableName>("variable") = chemical_potential;
    params.set<std::vector<VariableName>>("v") = {concentration};
    applyKernelParameters(params);
    _problem->addKernel("CoupledTimeDerivative", "kks_" + concentration + "_time", params);

    params = _factory.getValidParams("SplitCHWRes");
    params.set<NonlinearVariableName>("variable") = chemical_potential;
    params.set<MaterialPropertyName>("mob_name") = _concentration_mobilities[component];
    applyKernelParameters(params);
    _problem->addKernel("SplitCHWRes", "kks_" + concentration + "_diffusion", params);

    const auto component_phase_concentrations = phaseConcentrations(component);
    if (_phase_concentration_solve == PhaseConcentrationSolve::GLOBAL)
    {
      params = _factory.getValidParams("KKSSplitCHCRes");
      params.set<NonlinearVariableName>("variable") = concentration;
      params.set<MaterialPropertyName>("fa_name") = _free_energies[0];
      params.set<std::vector<VariableName>>("ca") = {component_phase_concentrations[0]};
      params.set<std::vector<VariableName>>("args_a") = phaseArguments(0, component);
      params.set<std::vector<VariableName>>("w") = {chemical_potential};
      applyKernelParameters(params);
      _problem->addKernel("KKSSplitCHCRes", "kks_" + concentration + "_chemical_potential", params);

      for (const auto phase : make_range(_phase_names.size() - 1))
      {
        params = _factory.getValidParams("KKSPhaseChemicalPotential");
        params.set<NonlinearVariableName>("variable") = component_phase_concentrations[phase];
        params.set<std::vector<VariableName>>("cb") = {component_phase_concentrations[phase + 1]};
        params.set<MaterialPropertyName>("fa_name") = _free_energies[phase];
        params.set<MaterialPropertyName>("fb_name") = _free_energies[phase + 1];
        params.set<std::vector<VariableName>>("args_a") = phaseArguments(phase, component);
        params.set<std::vector<VariableName>>("args_b") = phaseArguments(phase + 1, component);
        applyKernelParameters(params);
        _problem->addKernel("KKSPhaseChemicalPotential",
                            "kks_" + concentration + "_chemical_potential_" + _phase_names[phase] +
                                "_" + _phase_names[phase + 1],
                            params);
      }

      params = _factory.getValidParams("KKSMultiPhaseConcentration");
      params.set<NonlinearVariableName>("variable") = component_phase_concentrations.back();
      params.set<std::vector<VariableName>>("cj") = component_phase_concentrations;
      params.set<std::vector<VariableName>>("c") = {concentration};
      params.set<std::vector<VariableName>>("etas") = all_etas;
      params.set<std::vector<MaterialPropertyName>>("hj_names") = _switching_functions;
      applyKernelParameters(params);
      _problem->addKernel(
          "KKSMultiPhaseConcentration", "kks_" + concentration + "_conservation", params);
    }
    else
    {
      std::vector<MaterialPropertyName> first_phase_concentrations;
      first_phase_concentrations.reserve(_global_concentrations.size());
      for (const auto other_component : index_range(_global_concentrations))
        first_phase_concentrations.emplace_back(phaseConcentrations(other_component)[0]);

      params = _factory.getValidParams("NestedKKSMultiSplitCHCRes");
      params.set<NonlinearVariableName>("variable") = concentration;
      params.set<std::vector<VariableName>>("all_etas") = all_etas;
      params.set<std::vector<VariableName>>("global_cs") = global_cs;
      params.set<std::vector<VariableName>>("w") = {chemical_potential};
      params.set<std::vector<MaterialPropertyName>>("c1_names") = first_phase_concentrations;
      params.set<MaterialPropertyName>("F1_name") = _free_energies[0];
      applyKernelParameters(params);
      _problem->addKernel(
          "NestedKKSMultiSplitCHCRes", "kks_" + concentration + "_chemical_potential", params);
    }
  }

  for (const auto phase : index_range(_order_parameters))
  {
    const auto & eta = _order_parameters[phase];
    const auto coupled_variables = acCoupledVariables(phase);

    auto params = _factory.getValidParams("TimeDerivative");
    params.set<NonlinearVariableName>("variable") = eta;
    applyKernelParameters(params);
    _problem->addKernel("TimeDerivative", "kks_" + eta + "_time", params);

    params = _factory.getValidParams("ACInterface");
    params.set<NonlinearVariableName>("variable") = eta;
    params.set<MaterialPropertyName>("mob_name") = _order_parameter_mobilities[phase];
    params.set<MaterialPropertyName>("kappa_name") = _kappas[phase];
    params.set<std::vector<VariableName>>("coupled_variables") = coupled_variables;
    applyKernelParameters(params);
    _problem->addKernel("ACInterface", "kks_" + eta + "_interface", params);

    const auto bulk_f_type = _phase_concentration_solve == PhaseConcentrationSolve::GLOBAL
                                 ? "KKSMultiACBulkF"
                                 : "NestedKKSMultiACBulkF";
    params = _factory.getValidParams(bulk_f_type);
    params.set<NonlinearVariableName>("variable") = eta;
    params.set<std::vector<MaterialPropertyName>>("Fj_names") = _free_energies;
    params.set<std::vector<MaterialPropertyName>>("hj_names") = _switching_functions;
    params.set<std::vector<VariableName>>("eta_i") = {eta};
    params.set<MaterialPropertyName>("mob_name") = _order_parameter_mobilities[phase];
    params.set<Real>("wi") = _barrier_heights[phase];
    params.set<MaterialPropertyName>("gi_name") = _barrier_functions[phase];
    params.set<std::vector<VariableName>>("coupled_variables") = coupled_variables;
    if (_phase_concentration_solve == PhaseConcentrationSolve::NESTED)
    {
      params.set<std::vector<VariableName>>("global_cs") = global_cs;
      params.set<std::vector<VariableName>>("all_etas") = all_etas;
      params.set<std::vector<MaterialPropertyName>>("ci_names") = ci_names;
    }
    applyKernelParameters(params);
    _problem->addKernel(bulk_f_type, "kks_" + eta + "_bulk_free_energy", params);

    const auto num_bulk_c_kernels = _phase_concentration_solve == PhaseConcentrationSolve::GLOBAL
                                        ? _global_concentrations.size()
                                        : 1;
    for (const auto component : make_range(num_bulk_c_kernels))
    {
      const auto bulk_c_type = _phase_concentration_solve == PhaseConcentrationSolve::GLOBAL
                                   ? "KKSMultiACBulkC"
                                   : "NestedKKSMultiACBulkC";
      params = _factory.getValidParams(bulk_c_type);
      params.set<NonlinearVariableName>("variable") = eta;
      params.set<std::vector<MaterialPropertyName>>("Fj_names") = _free_energies;
      params.set<std::vector<MaterialPropertyName>>("hj_names") = _switching_functions;
      params.set<std::vector<VariableName>>("eta_i") = {eta};
      params.set<MaterialPropertyName>("mob_name") = _order_parameter_mobilities[phase];
      params.set<std::vector<VariableName>>("coupled_variables") = coupled_variables;
      if (_phase_concentration_solve == PhaseConcentrationSolve::GLOBAL)
        params.set<std::vector<VariableName>>("cj_names") = phaseConcentrations(component);
      else
      {
        params.set<std::vector<VariableName>>("global_cs") = global_cs;
        params.set<std::vector<VariableName>>("all_etas") = all_etas;
        params.set<std::vector<MaterialPropertyName>>("ci_names") = ci_names;
      }
      applyKernelParameters(params);
      const std::string kernel_suffix =
          _phase_concentration_solve == PhaseConcentrationSolve::GLOBAL
              ? std::string(_global_concentrations[component])
              : "phase_concentrations";
      _problem->addKernel(bulk_c_type, "kks_" + eta + "_bulk_" + kernel_suffix, params);
    }

    if (_phase_constraint == PhaseConstraint::LAGRANGE)
    {
      params = _factory.getValidParams("SwitchingFunctionConstraintEta");
      params.set<NonlinearVariableName>("variable") = eta;
      params.set<MaterialPropertyName>("h_name") = _switching_functions[phase];
      params.set<std::vector<VariableName>>("lambda") = {
          getParam<NonlinearVariableName>("lagrange_multiplier")};
      params.set<std::vector<VariableName>>("coupled_variables") = otherOrderParameters(phase);
      applyKernelParameters(params);
      _problem->addKernel("SwitchingFunctionConstraintEta", "kks_" + eta + "_constraint", params);
    }
  }

  if (_phase_constraint == PhaseConstraint::LAGRANGE)
  {
    auto params = _factory.getValidParams("SwitchingFunctionConstraintLagrange");
    params.set<NonlinearVariableName>("variable") =
        getParam<NonlinearVariableName>("lagrange_multiplier");
    params.set<std::vector<VariableName>>("etas") = all_etas;
    params.set<std::vector<MaterialPropertyName>>("h_names") = _switching_functions;
    params.set<Real>("epsilon") = getParam<Real>("constraint_epsilon");
    applyKernelParameters(params);
    _problem->addKernel("SwitchingFunctionConstraintLagrange", "kks_phase_constraint", params);
  }
}
