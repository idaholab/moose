/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PFCRFFKernelAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template<>
InputParameters validParams<PFCRFFKernelAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<unsigned int>("num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<std::string>("n_name", "Variable name used for the n variable");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used for n in this model");
  MooseEnum log_options("tolerance cancelation expansion");
  params.addRequiredParam<MooseEnum>("log_approach", log_options, "Which approach will be used to handle the natural log");
  params.addParam<Real>("tol", 1.0e-9, "Tolerance used when the tolerance approach is chosen");
  params.addParam<Real>("n_exp_terms", 4, "Number of terms used in the Taylor expansion of the natural log term");
  params.addParam<Real>("a", 1.0, "Parameter in the taylor series expansion");
  params.addParam<Real>("b", 1.0, "Parameter in the taylor series expansion");
  params.addParam<Real>("c", 1.0, "Parameter in the taylor series expansion");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");

  return params;
}

PFCRFFKernelAction::PFCRFFKernelAction(const InputParameters & params) :
    Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base")),
    _n_name(getParam<std::string>("n_name"))
{
}

void
PFCRFFKernelAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PFCRFFKernelAction Object\n";
  Moose::err << "L name base:" << _L_name_base;
#endif

  // Create the two kernels required for the n_variable, starting with the time derivative
  InputParameters poly_params = _factory.getValidParams("TimeDerivative");
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
  poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
  _problem->addKernel("TimeDerivative", "IE_n", poly_params);

  // First, we have to create the vector containing the names of the real L variables
  std::vector<VariableName> real_v(_num_L);
  for (unsigned int l = 0; l < _num_L; ++l)
    real_v[l] = _L_name_base + Moose::stringify(l) + "_real";

  // CHPFCRFF kernel
  poly_params = _factory.getValidParams("CHPFCRFF");
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
  poly_params.set<std::vector<VariableName> >("v") = real_v;
  poly_params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mob_name");
  poly_params.set<MooseEnum>("log_approach") = getParam<MooseEnum>("log_approach");
  poly_params.set<Real>("tol") = getParam<Real>("tol");
  poly_params.set<Real>("n_exp_terms") = getParam<Real>("n_exp_terms");
  poly_params.set<Real>("a") = getParam<Real>("a");
  poly_params.set<Real>("b") = getParam<Real>("b");
  poly_params.set<Real>("b") = getParam<Real>("c");
  poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  _problem->addKernel("CHPFCRFF", "CH_bulk_n", poly_params);

  // Loop over the L_variables
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    // Create L base name
    std::string L_name = _L_name_base + Moose::stringify(l);

    // Create real  and imaginary L variable names
    std::string real_name = L_name + "_real";
    std::string imag_name = L_name + "_imag";

    //
    // Create the kernels for the real L variable
    //

    // Create the diffusion kernel for L_real_l
    InputParameters poly_params = _factory.getValidParams("Diffusion");
    poly_params.set<NonlinearVariableName>("variable") = real_name;
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
    _problem->addKernel("Diffusion", "diff_" + real_name, poly_params);

    // Create the (alpha^R_m L^R_m) term
    poly_params = _factory.getValidParams("HHPFCRFF");
    poly_params.set<NonlinearVariableName>("variable") = real_name;
    poly_params.set<bool>("positive") = true;
    poly_params.set<MaterialPropertyName>("prop_name") = "alpha_R_" + Moose::stringify(l);
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
    _problem->addKernel("HHPFCRFF", "HH1_" + real_name, poly_params);

    // Create the -(alpha^I_m L^I_m) term
    if (l > 0)
    {
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = real_name;
      poly_params.set<bool>("positive") = false;
      poly_params.set<std::vector<VariableName> >("coupled_var").push_back(imag_name);
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      poly_params.set<MaterialPropertyName>("prop_name") = "alpha_I_" + Moose::stringify(l);
      _problem->addKernel("HHPFCRFF", "HH2_" + real_name, poly_params);
    }

    // Create the -(A^R_m n) term
    poly_params = _factory.getValidParams("HHPFCRFF");
    poly_params.set<NonlinearVariableName>("variable") = real_name;
    poly_params.set<bool>("positive") = false;
    poly_params.set<std::vector<VariableName> >("coupled_var").push_back(_n_name);
    poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
    poly_params.set<MaterialPropertyName>("prop_name") = "A_R_" + Moose::stringify(l);
    _problem->addKernel("HHPFCRFF", "HH3_" + real_name, poly_params);

    //
    // Create the kernels for the imaginary L variable, l > 0
    //
    if (l > 0)
    {
      // Create the diffusion kernel for L_imag_l
      InputParameters poly_params = _factory.getValidParams("Diffusion");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      _problem->addKernel("Diffusion", "diff_" + imag_name, poly_params);

      // **Create the (alpha^R_m L^I_m) term
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("positive") = true;
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      poly_params.set<MaterialPropertyName>("prop_name") = "alpha_R_" + Moose::stringify(l);
      _problem->addKernel("HHPFCRFF", "HH1_" + imag_name, poly_params);

      // **Create the (alpha^I_m L^R_m) term
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("positive") = true;
      poly_params.set<std::vector<VariableName> >("coupled_var").push_back(real_name);
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      poly_params.set<MaterialPropertyName>("prop_name") = "alpha_I_" + Moose::stringify(l);
      _problem->addKernel("HHPFCRFF", "HH2_" + imag_name, poly_params);

      // **Create the -(A^I_m n) term
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("positive") = false;
      poly_params.set<std::vector<VariableName> >("coupled_var").push_back(_n_name);
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      poly_params.set<MaterialPropertyName>("prop_name") = "A_I_" + Moose::stringify(l);
      _problem->addKernel("HHPFCRFF", "HH3_" + imag_name, poly_params);
    }
  }
}
