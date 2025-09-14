#include "AutomaticWeakFormAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "StringExpressionParser.h"
#include "MooseUtils.h"

registerMooseAction("MooseApp", AutomaticWeakFormAction, "create_problem_complete");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_variable");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_aux_variable");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_kernel");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_aux_kernel");
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
  // Check if we're using strong forms instead of energy functionals
  bool using_strong_forms =
      isParamValid("strong_forms") && !getParam<std::vector<std::string>>("strong_forms").empty();

  if (_current_task == "create_problem_complete")
  {
    unsigned int dim = _problem->mesh().dimension();
    _expr_builder = std::make_unique<moose::automatic_weak_form::MooseExpressionBuilder>(dim);
    _weak_form_gen = std::make_unique<moose::automatic_weak_form::WeakFormGenerator>(dim);
    _split_analyzer =
        std::make_unique<moose::automatic_weak_form::VariableSplittingAnalyzer>(_max_fe_order, dim);
    _mixed_gen = std::make_unique<moose::automatic_weak_form::MixedFormulationGenerator>();
    _problem_analyzer = std::make_unique<moose::automatic_weak_form::VariationalProblemAnalyzer>();
  }

  if (_current_task == "add_variable")
  {
    if (using_strong_forms)
    {
      parseStrongForms();
      deriveWeakForms();
    }
    else
    {
      parseEnergyFunctional();
    }
    analyzeVariables();
    addPrimaryVariables();
  }
  else if (_current_task == "add_aux_variable")
  {
    if (_enable_splitting)
      addSplitVariables();
  }
  else if (_current_task == "add_kernel")
  {
    if (using_strong_forms)
    {
      addTimeDerivativeKernels();
      addExpressionKernels();
    }
    else
    {
      addKernels();
    }
  }
  else if (_current_task == "add_aux_kernel")
  {
    if (_enable_splitting)
      addAuxKernels();
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
  for (const auto & var_name : _variable_names)
  {
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
    setupVariableSplitting();
}

void
AutomaticWeakFormAction::setupVariableSplitting()
{
  _split_variables = _split_analyzer->generateSplitVariables(_energy_functional);

  for (const auto & [name, sv] : _split_variables)
  {
    VariableInfo info;
    info.name = sv.name;
    info.family = "LAGRANGE";
    info.order = "FIRST";
    info.is_vector = std::holds_alternative<moose::automatic_weak_form::VectorShape>(sv.shape);
    info.is_tensor = std::holds_alternative<moose::automatic_weak_form::TensorShape>(sv.shape);
    info.components = info.is_vector ? _problem->mesh().dimension() : 1;
    info.is_auxiliary = true;
    info.is_split = true;
    info.parent_variable = sv.original_variable;

    _variable_info[sv.name] = info;
  }
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
  for (const auto & [name, sv] : _split_variables)
  {
    const auto & info = _variable_info[sv.name];

    if (_problem->hasVariable(sv.name))
      continue;

    InputParameters params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = info.family;
    params.set<MooseEnum>("order") = info.order;

    _problem->addAuxVariable("MooseVariable", sv.name, params);
  }
}

void
AutomaticWeakFormAction::addKernels()
{
  // Handle multiple energy expressions for coupled systems
  if (!_multiple_energies.empty())
  {
    for (const auto & [var_name, energy] : _multiple_energies)
    {
      auto weak_form = _weak_form_gen->generateWeakForm(energy, var_name);
      _weak_forms[var_name] = weak_form;
      generateKernelForVariable(var_name, weak_form);
    }
  }
  // Handle single energy expression
  else if (_energy_functional)
  {
    for (const auto & var_name : _variable_names)
    {
      auto weak_form = _weak_form_gen->generateWeakForm(_energy_functional, var_name);
      _weak_forms[var_name] = weak_form;
      generateKernelForVariable(var_name, weak_form);
    }
  }
}

void
AutomaticWeakFormAction::generateKernelForVariable(
    const std::string & var_name, const moose::automatic_weak_form::NodePtr & weak_form)
{
  std::string kernel_name = generateKernelName(var_name);

  InputParameters params = _factory.getValidParams("VariationalKernelBase");
  params.set<NonlinearVariableName>("variable") = var_name;
  params.set<MooseEnum>("energy_type") = "custom";
  params.set<bool>("use_automatic_differentiation") = _use_automatic_differentiation;
  params.set<bool>("compute_jacobian_numerically") = _compute_jacobian_numerically;
  params.set<Real>("fd_eps") = _fd_epsilon;
  params.set<bool>("enable_variable_splitting") = _enable_splitting;
  params.set<unsigned int>("fe_order") = _max_fe_order;

  if (!_coupled_variable_names.empty())
  {
    std::vector<VariableName> coupled_vars;
    for (const auto & name : _coupled_variable_names)
      coupled_vars.push_back(name);
    params.set<std::vector<VariableName>>("coupled_variables") = coupled_vars;
  }

  _problem->addKernel("VariationalKernelBase", kernel_name, params);
}

void
AutomaticWeakFormAction::addAuxKernels()
{
  moose::automatic_weak_form::SplitVariableKernelGenerator gen;
  auto kernel_infos = gen.generateKernels(_split_variables);

  for (const auto & info : kernel_infos)
  {
    InputParameters params = _factory.getValidParams("AuxKernel");
    params.set<AuxVariableName>("variable") = info.variable_name;

    for (const auto & coupled : info.coupled_variables)
      params.set<std::vector<VariableName>>("coupled") = {coupled};

    _problem->addAuxKernel("AuxKernel", info.kernel_name, params);
  }
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
