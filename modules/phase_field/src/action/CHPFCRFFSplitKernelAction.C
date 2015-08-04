#include "CHPFCRFFSplitKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<CHPFCRFFSplitKernelAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<unsigned int>("num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<NonlinearVariableName>("n_name", "Variable name used for the n variable");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used for n in this model");
  MooseEnum log_options("tolerance cancelation expansion");
  params.addRequiredParam<MooseEnum>("log_approach", log_options, "Which approach will be used to handle the natural log");
  params.addParam<Real>("tol", 1.0e-9, "Tolerance used when the tolerance approach is chosen");
  params.addParam<Real>("n_exp_terms", 4.0, "Number of terms used in the Taylor expansion of the natural log term");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

CHPFCRFFSplitKernelAction::CHPFCRFFSplitKernelAction(const InputParameters & params) :
    Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base")),
    _n_name(getParam<NonlinearVariableName>("n_name"))
{
}

void
CHPFCRFFSplitKernelAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the CHPFCRFFSplitKernelAction Object\n";
  Moose::err << "L name base:" << _L_name_base;
#endif

  // Create the two kernels required for the n_variable, starting with the time derivative
  InputParameters poly_params = _factory.getValidParams("TimeDerivative");
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
  poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  _problem->addKernel("TimeDerivative", "IE_n", poly_params);

  // Now the CHPFCRFF kernel
  std::vector<VariableName> real_v; // First, we have to create the vector containing the names of the real L variables
  real_v.resize(_num_L);
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    std::string L_name = _L_name_base;
    std::stringstream out;
    out << l;
    L_name.append(out.str());
    L_name.append("_real");
    real_v[l] = L_name;
  }

  poly_params = _factory.getValidParams("CHPFCRFF");
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
  poly_params.set<std::vector<VariableName> >("v") = real_v;
  poly_params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mob_name");
  poly_params.set<MooseEnum>("log_approach") = getParam<MooseEnum>("log_approach");
  poly_params.set<Real>("tol") = getParam<Real>("tol");
  poly_params.set<Real>("n_exp_terms") = getParam<Real>("n_exp_terms");
  poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  _problem->addKernel("CHPFCRFF", "CH_bulk_n", poly_params);

  // Loop over the L_variables
  // \todo This looks like it is not done yet
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    // Create L base name
    std::string L_name = _L_name_base;
    std::stringstream out;
    out << l;
    L_name.append(out.str());
  }
}

