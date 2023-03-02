//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HHPFCRFFSplitKernelAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("PhaseFieldApp", HHPFCRFFSplitKernelAction, "add_kernel");

InputParameters
HHPFCRFFSplitKernelAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set up kernels for the rational function fit (RFF) phase field crystal model");
  params.addRequiredParam<unsigned int>(
      "num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<VariableName>("n_name", "Variable name used for the n variable");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used for n in this model");
  MooseEnum log_options("tolerance cancelation expansion");
  params.addRequiredParam<MooseEnum>(
      "log_approach", log_options, "Which approach will be used to handle the natural log");
  params.addParam<Real>("tol", 1.0e-9, "Tolerance used when the tolerance approach is chosen");
  params.addParam<Real>(
      "n_exp_terms", 4, "Number of terms used in the Taylor expansion of the natural log term");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

HHPFCRFFSplitKernelAction::HHPFCRFFSplitKernelAction(const InputParameters & params)
  : Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base")),
    _n_name(getParam<VariableName>("n_name"))
{
}

void
HHPFCRFFSplitKernelAction::act()
{
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

    // **Create the -(alpha^I_m L^I_m) term
    if (l > 0)
    {
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = real_name;
      poly_params.set<bool>("positive") = false;
      poly_params.set<std::vector<VariableName>>("coupled_var").push_back(imag_name);
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      poly_params.set<MaterialPropertyName>("prop_name") = "alpha_I_" + Moose::stringify(l);
      _problem->addKernel("HHPFCRFF", "HH2_" + real_name, poly_params);
    }

    // **Create the -(A^R_m n) term
    poly_params = _factory.getValidParams("HHPFCRFF");
    poly_params.set<NonlinearVariableName>("variable") = real_name;
    poly_params.set<bool>("positive") = false;
    poly_params.set<std::vector<VariableName>>("coupled_var").push_back(_n_name);
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
      poly_params.set<std::vector<VariableName>>("coupled_var").push_back(real_name);
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      poly_params.set<MaterialPropertyName>("prop_name") = "alpha_I_" + Moose::stringify(l);
      _problem->addKernel("HHPFCRFF", "HH2_" + imag_name, poly_params);

      // **Create the -(A^I_m n) term
      poly_params = _factory.getValidParams("HHPFCRFF");
      poly_params.set<NonlinearVariableName>("variable") = imag_name;
      poly_params.set<bool>("positive") = false;
      poly_params.set<std::vector<VariableName>>("coupled_var").push_back(_n_name);
      poly_params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      poly_params.set<MaterialPropertyName>("prop_name") = "A_I_" + Moose::stringify(l);
      _problem->addKernel("HHPFCRFF", "HH3_" + imag_name, poly_params);
    }
  }
}
