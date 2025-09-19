#include "AutomaticWeakFormAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "StringExpressionParser.h"
#include "MooseUtils.h"
#include "ExpressionTransformer.h"

#include <algorithm>
#include <set>

registerMooseAction("MooseApp", AutomaticWeakFormAction, "create_problem_complete");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_variable");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_kernel");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_bc");

InputParameters
AutomaticWeakFormAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addClassDescription(
      "Automatically generates weak form from energy functionals or strong form equations");

  MooseEnum energy_types("expression double_well elastic_linear elastic_neohookean "
                         "surface_isotropic surface_anisotropic cahn_hilliard "
                         "cahn_hilliard_fourth_order phase_field_crystal navier_stokes custom");
  params.addRequiredParam<MooseEnum>("energy_type", energy_types, "Type of energy functional");

  params.addParam<std::string>(
      "energy_expression",
      "",
      "Mathematical expression for custom energy (e.g., 'W(c) + 0.5*kappa*dot(grad(c), grad(c))')");

  params.addParam<std::map<std::string, std::string>>(
      "energy_expressions",
      "Multiple energy expressions for coupled systems (var_name => expression)");

  params.addParam<std::string>(
      "expressions",
      "Intermediate expressions separated by semicolons (e.g., 'u = vec(disp_x, disp_y); strain = sym(grad(u))')");

  params.addParam<std::string>(
      "strong_forms",
      "Strong form equations separated by semicolons (e.g., 'c_t = -div(M*grad(mu)); mu = dW/dc - kappa*laplacian(c)')");

  params.addParam<std::map<std::string, Real>>(
      "parameters", "Parameters used in the energy expression (e.g., kappa=1.0)");

  params.addRequiredParam<std::vector<std::string>>("variables",
                                                    "Primary variables for the problem");

  params.addParam<std::vector<std::string>>("coupled_variables", "Additional coupled variables");

  params.addParam<bool>(
      "enable_splitting", true, "Enable automatic variable splitting for higher-order derivatives");

  params.addParam<unsigned int>("max_fe_order", 1, "Maximum finite element order available");

  params.addParam<bool>("use_automatic_differentiation", true, "Use AD for Jacobian computation");

  params.addParam<bool>(
      "compute_jacobian_numerically", false, "Use finite differences for Jacobian");

  params.addParam<Real>("fd_epsilon", 1e-8, "Finite difference epsilon");

  // Debugging and verbose output options
  params.addParam<bool>("verbose", false,
                         "Enable verbose output showing all processing steps and kernels added");
  params.addParam<bool>("debug_print_expressions", false,
                         "Print all parsed expressions and intermediate AST");
  params.addParam<bool>("debug_print_derivatives", false,
                         "Print detailed derivative calculations step by step");
  params.addParam<bool>("debug_print_simplification", false,
                         "Show expression simplification steps");
  params.addParam<bool>("debug_print_weak_form", false,
                         "Print final weak form expressions for each variable");
  params.addParam<bool>("debug_print_jacobian", false,
                         "Print Jacobian expressions for debugging convergence issues");

  params.addParam<bool>("add_time_derivative", false, "Add time derivative term");

  params.addParam<std::string>(
      "time_derivative_type", "first_order", "Type of time derivative (first_order, second_order)");

  params.addParam<bool>("enable_stabilization", false, "Enable stabilization for the weak form");

  params.addParam<std::string>(
      "stabilization_type", "SUPG", "Type of stabilization (SUPG, GLS, PSPG)");

  params.addParam<Real>("stabilization_parameter", 0.1, "Stabilization parameter value");

  params.addParam<bool>("symmetrize_jacobian", false, "Symmetrize the Jacobian matrix");

  params.addParam<bool>("use_preconditioner", true, "Use preconditioning");

  params.addParam<std::string>("preconditioner_type", "default", "Type of preconditioner");

  params.addParam<bool>("adaptive_splitting", false, "Use adaptive variable splitting");

  params.addParam<Real>("splitting_tolerance", 1e-3, "Tolerance for adaptive splitting");

  params.addParam<bool>("output_weak_form", false, "Output the generated weak form to file");

  params.addParam<std::string>("weak_form_file", "weak_form.txt", "File to output weak form");

  params.addParam<bool>("validate_conservation", false, "Check conservation properties");

  params.addParam<bool>("check_stability", false, "Perform stability analysis");

  params.addParam<bool>("estimate_condition_number", false, "Estimate condition number of system");

  params.addParam<Real>("gradient_coefficient", 1.0, "Gradient energy coefficient");
  params.addParam<Real>("fourth_order_coefficient", 0.0, "Fourth-order coefficient");
  params.addParam<Real>("elastic_lambda", 1.0, "First Lamé parameter");
  params.addParam<Real>("elastic_mu", 1.0, "Shear modulus");
  params.addParam<Real>("surface_energy", 1.0, "Surface energy coefficient");
  params.addParam<Real>("well_height", 1.0, "Double-well potential height");

  return params;
}

AutomaticWeakFormAction::AutomaticWeakFormAction(const InputParameters & params) : Action(params)
{
  // Initialize member variables
  _energy_expression = getParam<std::string>("energy_expression");
  _variable_names = getParam<std::vector<std::string>>("variables");
  _enable_splitting = getParam<bool>("enable_splitting");
  _max_fe_order = getParam<unsigned int>("max_fe_order");
  _use_automatic_differentiation = getParam<bool>("use_automatic_differentiation");
  _compute_jacobian_numerically = getParam<bool>("compute_jacobian_numerically");
  _fd_epsilon = getParam<Real>("fd_epsilon");
  _add_time_derivative = getParam<bool>("add_time_derivative");
  _time_derivative_type = getParam<std::string>("time_derivative_type");
  _enable_stabilization = getParam<bool>("enable_stabilization");
  _stabilization_type = getParam<std::string>("stabilization_type");
  _stabilization_parameter = getParam<Real>("stabilization_parameter");
  _symmetrize_jacobian = getParam<bool>("symmetrize_jacobian");
  _use_preconditioner = getParam<bool>("use_preconditioner");
  _preconditioner_type = getParam<std::string>("preconditioner_type");
  _adaptive_splitting = getParam<bool>("adaptive_splitting");
  _splitting_tolerance = getParam<Real>("splitting_tolerance");
  _output_weak_form = getParam<bool>("output_weak_form");
  _weak_form_file = getParam<std::string>("weak_form_file");
  _validate_conservation = getParam<bool>("validate_conservation");
  _check_stability = getParam<bool>("check_stability");
  _estimate_condition_number = getParam<bool>("estimate_condition_number");

  // Initialize debugging flags
  _verbose = getParam<bool>("verbose");
  _debug_print_expressions = getParam<bool>("debug_print_expressions");
  _debug_print_derivatives = getParam<bool>("debug_print_derivatives");
  _debug_print_simplification = getParam<bool>("debug_print_simplification");
  _debug_print_weak_form = getParam<bool>("debug_print_weak_form");
  _debug_print_jacobian = getParam<bool>("debug_print_jacobian");

  std::string type_str = getParam<MooseEnum>("energy_type");

  if (type_str == "expression")
    _energy_type = EnergyType::EXPRESSION;
  else if (type_str == "double_well")
    _energy_type = EnergyType::DOUBLE_WELL;
  else if (type_str == "elastic_linear")
    _energy_type = EnergyType::ELASTIC_LINEAR;
  else if (type_str == "elastic_neohookean")
    _energy_type = EnergyType::ELASTIC_NEOHOOKEAN;
  else if (type_str == "surface_isotropic")
    _energy_type = EnergyType::SURFACE_ISOTROPIC;
  else if (type_str == "surface_anisotropic")
    _energy_type = EnergyType::SURFACE_ANISOTROPIC;
  else if (type_str == "cahn_hilliard")
    _energy_type = EnergyType::CAHN_HILLIARD;
  else if (type_str == "cahn_hilliard_fourth_order")
    _energy_type = EnergyType::CAHN_HILLIARD_FOURTH_ORDER;
  else if (type_str == "phase_field_crystal")
    _energy_type = EnergyType::PHASE_FIELD_CRYSTAL;
  else if (type_str == "navier_stokes")
    _energy_type = EnergyType::NAVIER_STOKES;
  else
    _energy_type = EnergyType::CUSTOM;

  if (isParamValid("coupled_variables"))
    _coupled_variable_names = getParam<std::vector<std::string>>("coupled_variables");
}

void
AutomaticWeakFormAction::act()
{
  Moose::out << "\n[DEBUG] AutomaticWeakFormAction::act() called with task: " << _current_task << "\n";
  Moose::out << "  _enable_splitting = " << _enable_splitting << "\n";

  // Check if we're using strong forms instead of energy functionals
  bool using_strong_forms =
      isParamValid("strong_forms") && !getParam<std::string>("strong_forms").empty();

  if (_current_task == "create_problem_complete")
  {
    unsigned int dim = _problem->mesh().dimension();
    _expr_builder = std::make_unique<moose::automatic_weak_form::MooseExpressionBuilder>(dim);
    _weak_form_gen = std::make_unique<moose::automatic_weak_form::WeakFormGenerator>(dim);
    _split_analyzer =
        std::make_unique<moose::automatic_weak_form::VariableSplittingAnalyzer>(_max_fe_order, dim);
    _mixed_gen = std::make_unique<moose::automatic_weak_form::MixedFormulationGenerator>();
    _problem_analyzer = std::make_unique<moose::automatic_weak_form::VariationalProblemAnalyzer>();

    // Parse the energy functional early
    if (!using_strong_forms)
      parseEnergyFunctional();
  }

  if (_current_task == "add_variable")
  {
    Moose::out << "  [DEBUG] In add_variable task\n";
    if (using_strong_forms)
    {
      parseStrongForms();
      deriveWeakForms();
    }
    else if (!_energy_functional)
    {
      Moose::out << "  [DEBUG] Parsing energy functional\n";
      parseEnergyFunctional();
    }
    Moose::out << "  [DEBUG] Calling analyzeVariables()\n";
    analyzeVariables();
    Moose::out << "  [DEBUG] Calling addPrimaryVariables()\n";
    addPrimaryVariables();

    if (_enable_splitting)
    {
      // Need to re-analyze variables since action state doesn't persist across tasks
      if (!_energy_functional)
        parseEnergyFunctional();
      if (_split_variables.empty() && _energy_functional)
      {
        Moose::out << "  [DEBUG] Re-generating split variables in add_variable\n";
        _split_variables = _split_analyzer->generateSplitVariables(_energy_functional);
      }
      Moose::out << "  [DEBUG] add_variable: _split_variables.size() = " << _split_variables.size() << "\n";
      addSplitVariables();
    }
  }
  else if (_current_task == "add_kernel")
  {
    _console << "\n[AutomaticWeakFormAction] add_kernel task triggered\n";
    _console << "  using_strong_forms = " << (using_strong_forms ? "true" : "false") << "\n";
    _console << "  energy_type = " << static_cast<int>(_energy_type) << "\n";
    _console << "  energy_functional exists = " << (_energy_functional ? "yes" : "no") << "\n";

    if (using_strong_forms)
    {
      addTimeDerivativeKernels();
      addExpressionKernels();
    }
    else
    {
      _console << "  Calling addKernels()\n";
      addKernels();
    }
  }
  else if (_current_task == "add_bc")
  {
    addBoundaryConditions();
  }

  if (_output_weak_form && _current_task == "add_kernel")
    writeWeakFormToFile();

  if (_validate_conservation && _current_task == "add_kernel")
    performConservationCheck();

  if (_check_stability && _current_task == "add_kernel")
    performStabilityAnalysis();
}

void
AutomaticWeakFormAction::parseEnergyFunctional()
{
  buildEnergyFromType();

  if (!_energy_functional)
    mooseError("Failed to build energy functional");
}

void
AutomaticWeakFormAction::buildEnergyFromType()
{
  switch (_energy_type)
  {
    case EnergyType::EXPRESSION:
    {
      // Use the new string parser for arbitrary expressions
      moose::automatic_weak_form::StringExpressionParser parser(_problem->mesh().dimension());

      // Register parameters if provided
      if (isParamValid("parameters"))
      {
        const auto & params = getParam<std::map<std::string, Real>>("parameters");
        for (const auto & [name, value] : params)
          parser.setParameter(name, value);
      }

      // Define variables
      for (const auto & var_name : _variable_names)
        parser.defineVariable(var_name);
      for (const auto & var_name : _coupled_variable_names)
        parser.defineVariable(var_name);

      // Define intermediate expressions if provided
      if (isParamValid("expressions"))
      {
        const auto & expressions = getParam<std::vector<std::string>>("expressions");
        for (const auto & expr : expressions)
        {
          // Each expression should be of the form "name = expression"
          size_t eq_pos = expr.find('=');
          if (eq_pos != std::string::npos)
          {
            std::string name = expr.substr(0, eq_pos);
            std::string rhs = expr.substr(eq_pos + 1);

            // Trim whitespace
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            rhs.erase(0, rhs.find_first_not_of(" \t"));
            rhs.erase(rhs.find_last_not_of(" \t") + 1);

            parser.defineExpression(name, rhs);
          }
        }
      }

      // Handle multiple energy expressions for coupled systems
      if (isParamValid("energy_expressions"))
      {
        const auto & energy_exprs =
            getParam<std::map<std::string, std::string>>("energy_expressions");
        _multiple_energies = parser.parseMultipleEnergies(energy_exprs);
      }
      else if (!_energy_expression.empty())
      {
        // Single energy expression
        _energy_functional = parser.parse(_energy_expression);
      }
      else
      {
        mooseError(
            "Either energy_expression or energy_expressions must be provided for expression type");
      }
    }
    break;

    case EnergyType::DOUBLE_WELL:
      buildDoubleWellEnergy();
      break;

    case EnergyType::ELASTIC_LINEAR:
      buildElasticEnergy();
      break;

    case EnergyType::ELASTIC_NEOHOOKEAN:
      buildNeoHookeanEnergy();
      break;

    case EnergyType::SURFACE_ISOTROPIC:
      buildSurfaceEnergy();
      break;

    case EnergyType::CAHN_HILLIARD:
      buildCahnHilliardEnergy();
      break;

    case EnergyType::CAHN_HILLIARD_FOURTH_ORDER:
      buildFourthOrderCahnHilliardEnergy();
      break;

    case EnergyType::PHASE_FIELD_CRYSTAL:
      buildPhaseFieldCrystalEnergy();
      break;

    case EnergyType::NAVIER_STOKES:
      buildNavierStokesEnergy();
      break;

    default:
      mooseError("Unknown energy type");
  }
}

void
AutomaticWeakFormAction::buildDoubleWellEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto c = _expr_builder->field(_variable_names[0]);
  Real height = getParam<Real>("well_height");
  _energy_functional = _expr_builder->doubleWell(c, height);
}

void
AutomaticWeakFormAction::buildElasticEnergy()
{
  if (_variable_names.size() < _problem->mesh().dimension())
    mooseError("Elastic energy requires displacement components");

  std::string base_name = _variable_names[0];
  if (base_name.find("disp_x") != std::string::npos)
    base_name = "disp";

  auto strain = _expr_builder->strain(base_name);
  Real lambda = getParam<Real>("elastic_lambda");
  Real mu = getParam<Real>("elastic_mu");

  _energy_functional = _expr_builder->elasticEnergy(strain, lambda, mu);
}

void
AutomaticWeakFormAction::buildNeoHookeanEnergy()
{
  if (_variable_names.size() < _problem->mesh().dimension())
    mooseError("Neo-Hookean energy requires displacement components");

  std::string base_name = _variable_names[0];
  if (base_name.find("disp_x") != std::string::npos)
    base_name = "disp";

  auto F = _expr_builder->deformationGradient(base_name);
  Real lambda = getParam<Real>("elastic_lambda");
  Real mu = getParam<Real>("elastic_mu");

  _energy_functional = _expr_builder->neoHookean(F, mu, lambda);
}

void
AutomaticWeakFormAction::buildSurfaceEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto phi = _expr_builder->field(_variable_names[0]);
  Real gamma = getParam<Real>("surface_energy");

  _energy_functional = _expr_builder->surfaceEnergy(phi, gamma);
}

void
AutomaticWeakFormAction::buildCahnHilliardEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto c = _expr_builder->field(_variable_names[0]);
  Real kappa = getParam<Real>("gradient_coefficient");
  Real height = getParam<Real>("well_height");

  auto bulk = _expr_builder->doubleWell(c, height);
  auto grad_c = moose::automatic_weak_form::grad(c, _problem->mesh().dimension());
  auto gradient =
      moose::automatic_weak_form::multiply(moose::automatic_weak_form::constant(0.5 * kappa),
                                           moose::automatic_weak_form::dot(grad_c, grad_c));

  _energy_functional = moose::automatic_weak_form::add(bulk, gradient);
}

void
AutomaticWeakFormAction::buildFourthOrderCahnHilliardEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto c = _expr_builder->field(_variable_names[0]);
  Real kappa = getParam<Real>("gradient_coefficient");
  Real lambda = getParam<Real>("fourth_order_coefficient");
  Real height = getParam<Real>("well_height");

  auto bulk = _expr_builder->doubleWell(c, height);
  auto grad_c = moose::automatic_weak_form::grad(c, _problem->mesh().dimension());
  auto gradient =
      moose::automatic_weak_form::multiply(moose::automatic_weak_form::constant(0.5 * kappa),
                                           moose::automatic_weak_form::dot(grad_c, grad_c));
  auto fourth = _expr_builder->fourthOrderRegularization(c, lambda);

  _energy_functional =
      moose::automatic_weak_form::add(moose::automatic_weak_form::add(bulk, gradient), fourth);
}

void
AutomaticWeakFormAction::buildPhaseFieldCrystalEnergy()
{
  mooseError("Phase field crystal energy not yet implemented");
}

void
AutomaticWeakFormAction::buildNavierStokesEnergy()
{
  mooseError("Navier-Stokes energy not yet implemented");
}

void
AutomaticWeakFormAction::analyzeVariables()
{
  Moose::out << "  [DEBUG] analyzeVariables() called\n";
  Moose::out << "  [DEBUG] _variable_names.size() = " << _variable_names.size() << "\n";

  for (const auto & var_name : _variable_names)
  {
    Moose::out << "    [DEBUG] Processing variable: " << var_name << "\n";
    VariableInfo info;
    info.name = var_name;
    info.family = "LAGRANGE";
    info.order = "FIRST";
    info.is_vector = false;
    info.is_tensor = false;
    info.components = 1;
    info.is_auxiliary = false;
    info.is_split = false;

    _variable_info[var_name] = info;
  }

  if (_enable_splitting)
  {
    Moose::out << "  [DEBUG] Variable splitting is enabled, calling setupVariableSplitting()\n";
    setupVariableSplitting();
  }
  else
  {
    Moose::out << "  [DEBUG] Variable splitting is disabled\n";
  }
}

void
AutomaticWeakFormAction::setupVariableSplitting()
{
  Moose::out << "  [DEBUG] setupVariableSplitting() called\n";

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Analyzing expression for variable splitting...\n";

  Moose::out << "  [DEBUG] Energy functional before split analysis: " << (_energy_functional ? _energy_functional->toString() : "null") << "\n";

  if (!_energy_functional)
  {
    Moose::out << "  [DEBUG] ERROR: _energy_functional is null!\n";
    return;
  }

  _split_variables.clear();
  _split_plans.clear();
  _split_plan_order.clear();

  auto requirements = _split_analyzer->analyzeExpression(_energy_functional);
  auto generated_splits = _split_analyzer->generateSplitVariables(_energy_functional);

  Moose::out << "  [DEBUG] Number of candidate split variables: " << generated_splits.size() << "\n";

  moose::automatic_weak_form::HigherOrderSplittingStrategy strategy;

  auto strategyToString = [](moose::automatic_weak_form::HigherOrderSplittingStrategy::Strategy s) {
    using moose::automatic_weak_form::HigherOrderSplittingStrategy;
    switch (s)
    {
      case HigherOrderSplittingStrategy::Strategy::RECURSIVE: return "recursive";
      case HigherOrderSplittingStrategy::Strategy::DIRECT: return "direct";
      case HigherOrderSplittingStrategy::Strategy::MIXED: return "mixed";
      case HigherOrderSplittingStrategy::Strategy::OPTIMAL: return "optimal";
    }
    return "unknown";
  };

  for (const auto & req : requirements)
  {
    if (!req.requires_splitting)
      continue;

    auto plan = strategy.computeOptimalSplitting(
        _energy_functional,
        req.variable_name,
        req.max_derivative_order,
        _max_fe_order,
        generated_splits);

    if (plan.variables.empty())
      continue;

    auto [plan_it, inserted] = _split_plans.emplace(req.variable_name, plan);
    if (inserted)
      _split_plan_order.push_back(req.variable_name);
    else
      plan_it->second = plan;
  }

  if (_split_plans.empty())
  {
    Moose::out << "  [DEBUG] No split plans required\n";
    if (_verbose)
      _console << "  No variable splitting required (all derivatives within FE order "
               << _max_fe_order << ")\n";
    return;
  }

  std::map<std::string, moose::automatic_weak_form::SplitVariable> filtered_splits;

  for (const auto & [primary, plan] : _split_plans)
  {
    Moose::out << "    [DEBUG] Split plan for " << primary << " (strategy="
               << strategyToString(plan.strategy) << ")\n";

    if (_verbose)
      _console << "  Split plan for " << primary << " (" << plan.variables.size()
               << " variables, bandwidth " << plan.bandwidth << ")\n";

    for (const auto & plan_sv : plan.variables)
    {
      auto gen_it = generated_splits.find(plan_sv.name);
      if (gen_it == generated_splits.end())
      {
        Moose::out << "      [WARN] Missing generated split variable for " << plan_sv.name << "\n";
        continue;
      }

      auto sv = gen_it->second;

      auto [iter, inserted] = filtered_splits.emplace(sv.name, sv);

      Moose::out << "      [DEBUG] Using split variable: " << iter->second.name
                 << " (order " << iter->second.derivative_order << ", base "
                 << iter->second.original_variable << ") := "
                 << (iter->second.definition ? iter->second.definition->toString()
                                             : std::string("<undefined>"))
                 << "\n";

      if (_verbose)
        _console << "    - " << iter->second.name << " := "
                 << (iter->second.definition ? iter->second.definition->toString()
                                              : std::string("<undefined>"))
                 << "\n";

      registerSplitVariableInfo(iter->second, plan);
    }
  }

  _split_variables = std::move(filtered_splits);

  Moose::out << "  [DEBUG] Number of split variables after planning: " << _split_variables.size()
             << "\n";
}

void
AutomaticWeakFormAction::addPrimaryVariables()
{
  for (const auto & var_name : _variable_names)
  {
    const auto & info = _variable_info[var_name];

    if (_problem->hasVariable(var_name))
      continue;

    InputParameters params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = info.family;
    params.set<MooseEnum>("order") = info.order;

    _problem->addVariable("MooseVariable", var_name, params);
  }
}

void
AutomaticWeakFormAction::addSplitVariables()
{
  Moose::out << "  [DEBUG] addSplitVariables() called with " << _split_plans.size()
             << " split plans\n";

  if (_split_plans.empty())
    return;

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Adding split variables to nonlinear system\n";

  auto add_variable = [&](const moose::automatic_weak_form::SplitVariable & sv) {
    const auto & info = _variable_info.at(sv.name);

    if (_problem->hasVariable(sv.name))
    {
      Moose::out << "    [DEBUG] Variable " << sv.name << " already exists\n";
      return;
    }

    Moose::out << "    [DEBUG] Adding split variable: " << sv.name
               << " (components=" << info.components << ")\n";

    InputParameters params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = info.family;
    params.set<MooseEnum>("order") = info.order;
    _problem->addVariable("MooseVariable", sv.name, params);

    if (_verbose)
      _console << "  - Added split variable '" << sv.name
               << "' (derivative order " << sv.derivative_order << ")\n";
  };

  for (const auto & primary : _split_plan_order)
  {
    auto plan_it = _split_plans.find(primary);
    if (plan_it == _split_plans.end())
      continue;

    for (const auto & plan_sv : plan_it->second.variables)
    {
      auto split_it = _split_variables.find(plan_sv.name);
      if (split_it == _split_variables.end())
      {
        Moose::out << "    [WARN] Missing split variable info for " << plan_sv.name << "\n";
        continue;
      }

      if (_variable_info.find(plan_sv.name) == _variable_info.end())
        registerSplitVariableInfo(split_it->second, plan_it->second);

      add_variable(split_it->second);
    }
  }
}

void
AutomaticWeakFormAction::addKernels()
{
  // Rebuild splitting data if needed when transitioning between action stages
  if (_enable_splitting && (_split_variables.empty() || _split_plans.empty()))
  {
    if (!_energy_functional)
      parseEnergyFunctional();
    if (_energy_functional)
      setupVariableSplitting();
  }

  Moose::out << "  [DEBUG] AutomaticWeakFormAction::addKernels() called\n";
  Moose::out << "    _enable_splitting = " << _enable_splitting << "\n";
  Moose::out << "    _split_variables.size() = " << _split_variables.size() << "\n";

  if (_verbose)
    _console << "\n========================================\n"
             << "[AutomaticWeakForm] Adding Kernels\n"
             << "========================================\n";

  std::map<std::string, moose::automatic_weak_form::NodePtr> split_definitions;
  const bool have_split_defs = _enable_splitting && !_split_variables.empty();
  if (have_split_defs)
  {
    split_definitions = buildSplitDefinitionMap();
    _weak_form_gen->setSplitDefinitions(split_definitions);
  }

  // Check if this is a transient problem and add time derivative kernels
  bool is_transient = _problem->isTransient();

  if (_verbose)
    _console << "Problem type: " << (is_transient ? "TRANSIENT" : "STEADY-STATE") << "\n";

  // Handle multiple energy expressions for coupled systems
  if (!_multiple_energies.empty())
  {
    if (_verbose)
      _console << "\nProcessing multiple energy expressions for coupled system\n";

    for (const auto & [var_name, energy] : _multiple_energies)
    {
      if (_verbose)
        _console << "\n--- Processing variable: " << var_name << " ---\n";

      // Add time derivative kernel for transient problems
      if (is_transient)
        addTimeDerivativeKernelForVariable(var_name);

      // Transform the energy if we have split variables
      moose::automatic_weak_form::NodePtr transformed_energy = energy;
      if (_enable_splitting && !_split_variables.empty())
      {
        moose::automatic_weak_form::ExpressionTransformer transformer(_split_variables, _problem->mesh().dimension());
        transformed_energy = transformer.transform(energy);

        if (_verbose && transformed_energy != energy)
          _console << "  Transformed energy for split variables\n";
      }

      // Debug: Check shape of energy before generating weak form
      if (true) // Always debug for now
      {
        _console << "  [DEBUG] About to generate weak form for " << var_name << "\n";
        _console << "  Energy expression: " << transformed_energy->toString() << "\n";

        // Try to find grad(grad(c)) in the expression tree
        std::function<void(const moose::automatic_weak_form::NodePtr&)> checkShape;
        checkShape = [&](const moose::automatic_weak_form::NodePtr& node) {
          if (node->type() == moose::automatic_weak_form::NodeType::Gradient)
          {
            auto unary = static_cast<const moose::automatic_weak_form::UnaryOpNode*>(node.get());
            if (unary->operand()->type() == moose::automatic_weak_form::NodeType::Gradient)
            {
              _console << "  Found grad(grad(...)) with shape: ";
              if (node->isVector())
              {
                auto shape = node->shape();
                if (std::holds_alternative<moose::automatic_weak_form::VectorShape>(shape))
                  _console << "VECTOR(dim=" << std::get<moose::automatic_weak_form::VectorShape>(shape).dim << ")\n";
                else
                  _console << "VECTOR(unknown)\n";
              }
              else if (node->isTensor())
              {
                auto shape = node->shape();
                if (std::holds_alternative<moose::automatic_weak_form::TensorShape>(shape))
                  _console << "TENSOR(dim=" << std::get<moose::automatic_weak_form::TensorShape>(shape).dim << ")\n";
                else
                  _console << "TENSOR(unknown)\n";
              }
              else
                _console << "UNKNOWN\n";
            }
          }

          // Recursively check children
          for (const auto& child : node->children())
            if (child)
              checkShape(child);
        };

        checkShape(transformed_energy);
      }

      auto weak_form = _weak_form_gen->generateWeakForm(transformed_energy, var_name);
      _weak_forms[var_name] = weak_form;

      if (_debug_print_weak_form && weak_form)
        _console << "  Weak form: " << weak_form->toString() << "\n";

      generateKernelForVariable(var_name, weak_form, transformed_energy);
    }
  }
  // Handle single energy expression
  else if (_energy_functional)
  {
    if (_verbose)
      _console << "\nProcessing single energy functional\n";

    if (_debug_print_expressions)
      _console << "  Energy: " << _energy_functional->toString() << "\n";

    for (const auto & var_name : _variable_names)
    {
      if (_verbose)
        _console << "\n--- Processing variable: " << var_name << " ---\n";

      // Add time derivative kernel for transient problems
      if (is_transient)
        addTimeDerivativeKernelForVariable(var_name);

      // Transform the energy if we have split variables
      moose::automatic_weak_form::NodePtr transformed_energy = _energy_functional;
      if (_enable_splitting && !_split_variables.empty())
      {
        Moose::out << "    [DEBUG] Transforming energy with " << _split_variables.size() << " split variables\n";
        for (const auto & [name, sv] : _split_variables)
        {
          Moose::out << "      Split var: " << name << " = " << sv.name << " (order " << sv.derivative_order << ")\n";
        }
        moose::automatic_weak_form::ExpressionTransformer transformer(_split_variables, _problem->mesh().dimension());
        transformed_energy = transformer.transform(_energy_functional);

        Moose::out << "    [DEBUG] Original energy: " << _energy_functional->toString() << "\n";
        Moose::out << "    [DEBUG] Transformed energy: " << transformed_energy->toString() << "\n";

        if (_verbose && transformed_energy != _energy_functional)
          _console << "  Transformed energy functional for split variables\n";
      }
      else
      {
        Moose::out << "    [DEBUG] Not transforming - splitting=" << _enable_splitting
                   << ", split_vars=" << _split_variables.size() << "\n";
      }

      // Debug: Check shape of energy before generating weak form
      if (true) // Always debug for now
      {
        _console << "  [DEBUG] About to generate weak form for " << var_name << "\n";
        _console << "  Energy expression: " << transformed_energy->toString() << "\n";

        // Try to find grad(grad(c)) in the expression tree
        std::function<void(const moose::automatic_weak_form::NodePtr&)> checkShape;
        checkShape = [&](const moose::automatic_weak_form::NodePtr& node) {
          if (node->type() == moose::automatic_weak_form::NodeType::Gradient)
          {
            auto unary = static_cast<const moose::automatic_weak_form::UnaryOpNode*>(node.get());
            if (unary->operand()->type() == moose::automatic_weak_form::NodeType::Gradient)
            {
              _console << "  Found grad(grad(...)) with shape: ";
              if (node->isVector())
              {
                auto shape = node->shape();
                if (std::holds_alternative<moose::automatic_weak_form::VectorShape>(shape))
                  _console << "VECTOR(dim=" << std::get<moose::automatic_weak_form::VectorShape>(shape).dim << ")\n";
                else
                  _console << "VECTOR(unknown)\n";
              }
              else if (node->isTensor())
              {
                auto shape = node->shape();
                if (std::holds_alternative<moose::automatic_weak_form::TensorShape>(shape))
                  _console << "TENSOR(dim=" << std::get<moose::automatic_weak_form::TensorShape>(shape).dim << ")\n";
                else
                  _console << "TENSOR(unknown)\n";
              }
              else
                _console << "UNKNOWN\n";
            }
          }

          // Recursively check children
          for (const auto& child : node->children())
            if (child)
              checkShape(child);
        };

        checkShape(transformed_energy);
      }

      auto weak_form = _weak_form_gen->generateWeakForm(transformed_energy, var_name);
      _weak_forms[var_name] = weak_form;

      // Always print for info messages (existing behavior)
      _console << "Generated weak form for " << var_name << ": ";
      if (weak_form)
        _console << weak_form->toString() << "\n";
      else
        _console << "null\n";

      generateKernelForVariable(var_name, weak_form, transformed_energy);
    }
  }

  if (have_split_defs)
    _weak_form_gen->clearSplitDefinitions();

  if (_enable_splitting && !_split_variables.empty())
    addSplitConstraintKernels();

  if (_verbose)
    _console << "\n========================================\n"
             << "[AutomaticWeakForm] Kernel Addition Complete\n"
             << "========================================\n";
}

void
AutomaticWeakFormAction::addSplitConstraintKernels()
{
  if (_split_variables.empty())
    return;

  auto split_definitions = buildSplitDefinitionMap();
  _weak_form_gen->setSplitDefinitions(split_definitions);

  std::set<std::string> processed;

  for (const auto & primary : _split_plan_order)
  {
    auto plan_it = _split_plans.find(primary);
    if (plan_it == _split_plans.end())
      continue;

    for (const auto & plan_sv : plan_it->second.variables)
    {
      auto split_it = _split_variables.find(plan_sv.name);
      if (split_it == _split_variables.end())
        continue;

      const auto & sv = split_it->second;
      if (!sv.constraint_residual)
        continue;

      auto energy = buildConstraintEnergy(sv);
      if (!energy)
        continue;

      auto weak_form = _weak_form_gen->generateWeakForm(energy, sv.name);
      _weak_forms[sv.name] = weak_form;

      generateKernelForVariable(sv.name, weak_form, energy, true);
      processed.insert(sv.name);
    }
  }

  for (const auto & [name, sv] : _split_variables)
  {
    if (processed.count(name))
      continue;
    if (!sv.constraint_residual)
      continue;

    auto energy = buildConstraintEnergy(sv);
    if (!energy)
      continue;

    auto weak_form = _weak_form_gen->generateWeakForm(energy, sv.name);
    _weak_forms[sv.name] = weak_form;
    generateKernelForVariable(sv.name, weak_form, energy, true);
  }

  _weak_form_gen->clearSplitDefinitions();
}

void
AutomaticWeakFormAction::addTimeDerivativeKernelForVariable(const std::string & var_name)
{
  std::string kernel_name = var_name + "_time_derivative";

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Adding time derivative kernel for variable: " << var_name << "\n";

  // Use the specialized VariationalTimeDerivative kernel if automatic differentiation is enabled
  if (_use_automatic_differentiation)
  {
    if (_verbose)
      _console << "  - Using VariationalTimeDerivative with automatic differentiation\n";

    InputParameters params = _factory.getValidParams("VariationalTimeDerivative");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<Real>("coefficient") = 1.0;  // Default coefficient, could be made configurable
    params.set<bool>("use_automatic_differentiation") = true;
    _problem->addKernel("VariationalTimeDerivative", kernel_name, params);
  }
  else
  {
    if (_verbose)
      _console << "  - Using standard TimeDerivative kernel\n";

    // Use standard TimeDerivative kernel
    InputParameters params = _factory.getValidParams("TimeDerivative");
    params.set<NonlinearVariableName>("variable") = var_name;
    _problem->addKernel("TimeDerivative", kernel_name, params);
  }

  if (_verbose)
    _console << "  - Kernel name: " << kernel_name << "\n";
}

std::map<std::string, moose::automatic_weak_form::NodePtr>
AutomaticWeakFormAction::buildSplitDefinitionMap() const
{
  std::map<std::string, moose::automatic_weak_form::NodePtr> defs;
  for (const auto & [name, sv] : _split_variables)
    if (sv.definition)
      defs[name] = sv.definition;
  return defs;
}

unsigned int
AutomaticWeakFormAction::computeSplitComponents(const moose::automatic_weak_form::Shape & shape) const
{
  using namespace moose::automatic_weak_form;

  if (std::holds_alternative<ScalarShape>(shape))
    return 1u;
  if (const auto * vec = std::get_if<VectorShape>(&shape))
    return vec->dim;
  if (const auto * tensor = std::get_if<TensorShape>(&shape))
    return tensor->dim * tensor->dim;
  if (const auto * rank3 = std::get_if<RankThreeShape>(&shape))
    return rank3->dim * rank3->dim * rank3->dim;
  if (const auto * rank4 = std::get_if<RankFourShape>(&shape))
    return rank4->dim1 * rank4->dim2;

  return 1u;
}

void
AutomaticWeakFormAction::registerSplitVariableInfo(
    const moose::automatic_weak_form::SplitVariable & sv,
    const moose::automatic_weak_form::HigherOrderSplittingStrategy::SplitPlan & plan)
{
  VariableInfo info;
  info.name = sv.name;
  info.family = "LAGRANGE";
  info.order = "FIRST";
  info.is_vector = std::holds_alternative<moose::automatic_weak_form::VectorShape>(sv.shape);
  info.is_tensor = std::holds_alternative<moose::automatic_weak_form::TensorShape>(sv.shape) ||
                   std::holds_alternative<moose::automatic_weak_form::RankThreeShape>(sv.shape) ||
                   std::holds_alternative<moose::automatic_weak_form::RankFourShape>(sv.shape);
  info.components = computeSplitComponents(sv.shape);
  info.is_auxiliary = false;
  info.is_split = true;
  info.parent_variable = sv.original_variable;

  for (const auto & dep : plan.dependencies)
    if (dep.first == sv.name)
    {
      info.parent_variable = dep.second;
      break;
    }

  _variable_info[sv.name] = info;
}

moose::automatic_weak_form::NodePtr
AutomaticWeakFormAction::buildConstraintEnergy(
    const moose::automatic_weak_form::SplitVariable & sv) const
{
  using namespace moose::automatic_weak_form;

  if (!sv.constraint_residual)
    return nullptr;

  NodePtr quadratic = nullptr;

  if (std::holds_alternative<ScalarShape>(sv.shape))
    quadratic = multiply(sv.constraint_residual, sv.constraint_residual);
  else if (std::holds_alternative<VectorShape>(sv.shape))
    quadratic = dot(sv.constraint_residual, sv.constraint_residual);
  else if (std::holds_alternative<TensorShape>(sv.shape) ||
           std::holds_alternative<RankThreeShape>(sv.shape) ||
           std::holds_alternative<RankFourShape>(sv.shape))
    quadratic = contract(sv.constraint_residual, sv.constraint_residual);
  else
    quadratic = multiply(sv.constraint_residual, sv.constraint_residual);

  return multiply(constant(0.5), quadratic);
}

void
AutomaticWeakFormAction::generateKernelForVariable(
    const std::string & var_name,
    const moose::automatic_weak_form::NodePtr & weak_form,
    const moose::automatic_weak_form::NodePtr & energy_expr,
    bool force_custom_energy)
{
  (void)weak_form;
  std::string kernel_name = generateKernelName(var_name);

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Generating kernel for variable: " << var_name << "\n"
             << "  - Kernel name: " << kernel_name << "\n"
             << "  - Kernel type: VariationalKernelBase\n";

  InputParameters params = _factory.getValidParams("VariationalKernelBase");

  // First, we need to couple all the other variables BEFORE setting parameters
  // This is required for MOOSE's coupling system to work
  // We do this by modifying the validParams to add coupled variables
  // But we can't modify validParams after getting them, so we need a different approach

  // The VariationalKernelBase should handle coupling internally based on the
  // coupled_variables parameter we'll pass

  params.set<NonlinearVariableName>("variable") = var_name;

  // Get the energy type string from the input
  const auto & energy_type_enum = getParam<MooseEnum>("energy_type");
  if (force_custom_energy || _energy_type == EnergyType::EXPRESSION)
    params.set<MooseEnum>("energy_type") = "custom";
  else
    params.set<MooseEnum>("energy_type") = energy_type_enum;
  params.set<bool>("use_automatic_differentiation") = _use_automatic_differentiation;
  params.set<bool>("compute_jacobian_numerically") = _compute_jacobian_numerically;
  params.set<Real>("fd_eps") = _fd_epsilon;
  params.set<bool>("enable_variable_splitting") = force_custom_energy ? false : _enable_splitting;
  params.set<unsigned int>("fe_order") = _max_fe_order;

  // Pass the energy expression - use transformed if splitting was applied
  if (_energy_type == EnergyType::EXPRESSION)
  {
    // If we have a transformed energy expression, convert it to string
    if (energy_expr)
    {
      std::string transformed_expr = energy_expr->toString();
      params.set<std::string>("energy_expression") = transformed_expr;
      if (_verbose)
        _console << "  - Passing transformed energy expression to kernel: " << transformed_expr << "\n";
    }
    else
    {
      params.set<std::string>("energy_expression") = _energy_expression;
      if (_verbose)
        _console << "  - Passing original energy expression to kernel: " << _energy_expression << "\n";
    }

    // Pass parameters if any
    if (isParamValid("parameters"))
      params.set<std::map<std::string, Real>>("parameters") =
          getParam<std::map<std::string, Real>>("parameters");
  }

  // For coupled problems, each variable's kernel needs the OTHER variables as coupled
  std::vector<VariableName> coupled_vars;

  // Add all variables from the energy expression except the current one
  for (const auto & name : _variable_names)
  {
    if (name != var_name)
      coupled_vars.push_back(name);
  }

  // Also add any explicitly specified coupled variables
  for (const auto & name : _coupled_variable_names)
  {
    // Don't add if it's the current variable or already in the list
    if (name != var_name && std::find(coupled_vars.begin(), coupled_vars.end(), name) == coupled_vars.end())
      coupled_vars.push_back(name);
  }

  for (const auto & [split_name, _] : _split_variables)
  {
    if (split_name == var_name)
      continue;
    if (std::find(coupled_vars.begin(), coupled_vars.end(), split_name) == coupled_vars.end())
      coupled_vars.push_back(split_name);
  }

  if (!coupled_vars.empty())
  {
    // Set the coupled variables
    params.set<std::vector<VariableName>>("coupled_vars") = coupled_vars;

    if (_verbose)
    {
      _console << "  - Coupled variables: ";
      for (size_t i = 0; i < _coupled_variable_names.size(); ++i)
      {
        if (i > 0) _console << ", ";
        _console << _coupled_variable_names[i];
      }
      _console << "\n";
    }
  }

  if (_verbose)
  {
    _console << "  - Automatic differentiation: " << (_use_automatic_differentiation ? "ENABLED" : "DISABLED") << "\n";
    if (_enable_splitting)
      _console << "  - Variable splitting: ENABLED (max FE order: " << _max_fe_order << ")\n";
  }

  _problem->addKernel("VariationalKernelBase", kernel_name, params);

  if (_verbose)
    _console << "  - Kernel successfully added to problem\n";
}

void
AutomaticWeakFormAction::addAuxKernels()
{
  if (_verbose)
    _console << "\n[AutomaticWeakForm] Adding auxiliary kernels for split variables\n";

  // For each split variable, add an auxiliary kernel to compute it
  for (const auto & [name, sv] : _split_variables)
  {
    if (_verbose)
      _console << "  - Adding kernel for split variable: " << sv.name
               << " (order " << sv.derivative_order << " derivative of "
               << sv.original_variable << ")\n";

    // For gradient computation (order 1), use GradientAuxKernel
    if (sv.derivative_order == 1)
    {
      // Check if the split variable is a vector (gradient of scalar)
      if (std::holds_alternative<moose::automatic_weak_form::VectorShape>(sv.shape))
      {
        // For vector auxiliary variables, we need to add component kernels
        std::vector<std::string> component_names = {"x", "y", "z"};
        for (unsigned int comp = 0; comp < _problem->mesh().dimension(); ++comp)
        {
          std::string kernel_name = sv.name + "_" + component_names[comp] + "_gradient";
          InputParameters params = _factory.getValidParams("GradientAuxKernel");

          // Set the vector auxiliary variable and component
          params.set<AuxVariableName>("variable") = sv.name;
          params.set<std::vector<VariableName>>("coupled_variable") = {sv.original_variable};
          params.set<unsigned int>("component") = comp;

          _problem->addAuxKernel("GradientAuxKernel", kernel_name, params);

          if (_verbose)
            _console << "    Added component " << component_names[comp] << " kernel: " << kernel_name << "\n";
        }
      }
    }
    // For higher-order derivatives, we'd need specialized kernels
    else if (sv.derivative_order == 2)
    {
      // This would compute the gradient of a gradient (Hessian)
      // Would need a HessianAuxKernel or similar
      if (_verbose)
        _console << "    WARNING: Second-order derivatives not yet fully implemented\n";
    }
  }

  if (_verbose && _split_variables.empty())
    _console << "  No split variables needed (all derivatives within FE order)\n";
}

void
AutomaticWeakFormAction::addBoundaryConditions()
{
}

std::string
AutomaticWeakFormAction::generateKernelName(const std::string & var_name,
                                            const std::string & suffix)
{
  std::string name = var_name + "_variational";
  if (!suffix.empty())
    name += "_" + suffix;
  return name;
}

void
AutomaticWeakFormAction::writeWeakFormToFile()
{
  std::ofstream file(_weak_form_file);

  if (!file.is_open())
  {
    mooseWarning("Could not open weak form output file: ", _weak_form_file);
    return;
  }

  file << "# Automatically generated weak form\n";
  file << "# Energy type: " << getParam<MooseEnum>("energy_type") << "\n";
  file << "# Variables: ";
  for (const auto & var : _variable_names)
    file << var << " ";
  file << "\n\n";

  for (const auto & [var_name, weak_form] : _weak_forms)
  {
    file << "# Weak form for variable: " << var_name << "\n";
    file << weak_form->toString() << "\n\n";
  }

  if (!_split_variables.empty())
  {
    file << "# Split variables:\n";
    for (const auto & [name, sv] : _split_variables)
    {
      file << "# " << sv.toString() << "\n";
    }
  }

  file.close();
}

void
AutomaticWeakFormAction::performConservationCheck()
{
  for (const auto & [var_name, weak_form] : _weak_forms)
  {
    auto check = _weak_form_gen->checkConservation(weak_form, var_name);

    if (check.is_conservative)
      _console << "Variable " << var_name << " is conservative\n";
    else
      _console << "Variable " << var_name << " may not be conservative: " << check.message << "\n";
  }
}

void
AutomaticWeakFormAction::performStabilityAnalysis()
{
  auto analysis = _problem_analyzer->analyze(_energy_functional, _variable_names);

  _console << "\nStability Analysis:\n";
  _console << "  Well-posed: " << (analysis.is_well_posed ? "Yes" : "No") << "\n";
  _console << "  Elliptic: " << (analysis.is_elliptic ? "Yes" : "No") << "\n";
  _console << "  Parabolic: " << (analysis.is_parabolic ? "Yes" : "No") << "\n";
  _console << "  Required continuity: C^" << analysis.required_continuity << "\n";
  _console << "  Recommended FE order: " << analysis.recommended_fe_order << "\n";

  if (!analysis.warnings.empty())
  {
    _console << "  Warnings:\n";
    for (const auto & warning : analysis.warnings)
      _console << "    - " << warning << "\n";
  }

  if (!analysis.suggestions.empty())
  {
    _console << "  Suggestions:\n";
    for (const auto & suggestion : analysis.suggestions)
      _console << "    - " << suggestion << "\n";
  }
}

void
AutomaticWeakFormAction::parseStrongForms()
{
  // Create parser
  moose::automatic_weak_form::StringExpressionParser parser(_problem->mesh().dimension());

  // Set up parameters
  if (isParamValid("parameters"))
  {
    const auto & params = getParam<std::map<std::string, Real>>("parameters");
    for (const auto & [name, value] : params)
      parser.setParameter(name, value);
  }

  // Define all variables
  for (const auto & var_name : _variable_names)
    parser.defineVariable(var_name);
  for (const auto & var_name : _coupled_variable_names)
    parser.defineVariable(var_name);

  // Parse intermediate expressions if provided
  if (isParamValid("expressions"))
  {
    const std::string & expr_string = getParam<std::string>("expressions");

    // Split by semicolons
    std::vector<std::string> expressions;
    MooseUtils::tokenize(expr_string, expressions, 1, ";");

    for (const auto & expr : expressions)
    {
      // Skip empty expressions
      if (expr.empty())
        continue;

      // Each expression should be of the form "name = expression"
      size_t eq_pos = expr.find('=');
      if (eq_pos != std::string::npos)
      {
        std::string name = expr.substr(0, eq_pos);
        std::string rhs = expr.substr(eq_pos + 1);

        // Trim whitespace
        name.erase(0, name.find_first_not_of(" \t\n"));
        name.erase(name.find_last_not_of(" \t\n") + 1);
        rhs.erase(0, rhs.find_first_not_of(" \t\n"));
        rhs.erase(rhs.find_last_not_of(" \t\n") + 1);

        // Skip if either name or rhs is empty after trimming
        if (name.empty() || rhs.empty())
          continue;

        parser.defineExpression(name, rhs);
      }
    }
  }

  // Parse strong form equations
  if (isParamValid("strong_forms"))
  {
    const std::string & forms_string = getParam<std::string>("strong_forms");

    // The string contains semicolon-separated equations
    _strong_form_equations = parser.parseStrongForms(forms_string);
  }
}

void
AutomaticWeakFormAction::deriveWeakForms()
{
  // For each strong form equation, derive the weak form
  // This involves:
  // 1. Multiplying by test function
  // 2. Integrating by parts if needed
  // 3. Computing Jacobian terms

  for (auto & [var_name, eq] : _strong_form_equations)
  {
    // The weak form residual for equation du/dt = F or 0 = F is:
    // R = ∫ test * (F) dx  (after appropriate integration by parts)

    // For now, we store the RHS as the weak residual
    // A more complete implementation would perform integration by parts
    // on divergence terms and properly handle boundary conditions

    eq.weak_residual = eq.rhs;

    // TODO: Compute Jacobian symbolically or mark for AD
    eq.jacobian = nullptr;
  }
}

void
AutomaticWeakFormAction::addTimeDerivativeKernels()
{
  // Add TimeDerivative kernels for transient terms
  for (const auto & [var_name, eq] : _strong_form_equations)
  {
    if (eq.type == moose::automatic_weak_form::StringExpressionParser::EquationType::TRANSIENT)
    {
      std::string kernel_name = var_name + "_time_derivative";

      InputParameters params = _factory.getValidParams("TimeDerivative");
      params.set<NonlinearVariableName>("variable") = var_name;

      _problem->addKernel("TimeDerivative", kernel_name, params);
    }
  }
}

void
AutomaticWeakFormAction::addExpressionKernels()
{
  // Add ExpressionEvaluationKernel for each equation's residual
  for (const auto & [var_name, eq] : _strong_form_equations)
  {
    std::string kernel_name = var_name + "_expression_kernel";

    InputParameters params = _factory.getValidParams("ExpressionEvaluationKernel");
    params.set<NonlinearVariableName>("variable") = var_name;

    // For now, just pass the raw RHS expression as a string
    // The weak_residual would need proper conversion
    // TODO: Implement proper expression serialization
    std::string residual_expr = "0";  // Placeholder - need to convert eq.rhs properly
    params.set<std::string>("residual_expression") = residual_expr;

    if (eq.jacobian)
      params.set<std::string>("jacobian_expression") = eq.jacobian->toString();

    params.set<bool>("is_transient_term") =
        (eq.type == moose::automatic_weak_form::StringExpressionParser::EquationType::TRANSIENT);

    // Pass parameters
    if (isParamValid("parameters"))
      params.set<std::map<std::string, Real>>("parameters") =
          getParam<std::map<std::string, Real>>("parameters");

    // Pass intermediate expressions
    if (isParamValid("expressions"))
    {
      const std::string & expressions = getParam<std::string>("expressions");
      // Just pass the string directly - it already has semicolons
      if (!expressions.empty())
        params.set<std::string>("intermediate_expressions") = expressions;
    }

    // Pass coupled variables
    if (!_coupled_variable_names.empty())
    {
      std::vector<VariableName> coupled_vars;
      for (const auto & name : _coupled_variable_names)
        coupled_vars.push_back(name);
      params.set<std::vector<VariableName>>("coupled") = coupled_vars;
    }

    params.set<bool>("use_automatic_differentiation") = _use_automatic_differentiation;

    _problem->addKernel("ExpressionEvaluationKernel", kernel_name, params);
  }
}
