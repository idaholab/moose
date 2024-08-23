//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHPFCRFFSplitKernelAction.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"

registerMooseAction("PhaseFieldApp", CHPFCRFFSplitKernelAction, "add_kernel");

InputParameters
CHPFCRFFSplitKernelAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Creates the kernels for the transient Cahn-Hilliard equation for the "
                             "RFF form of the phase field crystal model");
  params.addRequiredParam<unsigned int>(
      "num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<NonlinearVariableName>("n_name", "Variable name used for the n variable");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used for n in this model");
  MooseEnum log_options("tolerance cancelation expansion");
  params.addRequiredParam<MooseEnum>(
      "log_approach", log_options, "Which approach will be used to handle the natural log");
  params.addParam<Real>("tol", 1.0e-9, "Tolerance used when the tolerance approach is chosen");
  params.addParam<Real>(
      "n_exp_terms", 4.0, "Number of terms used in the Taylor expansion of the natural log term");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

CHPFCRFFSplitKernelAction::CHPFCRFFSplitKernelAction(const InputParameters & params)
  : Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base")),
    _n_name(getParam<NonlinearVariableName>("n_name"))
{
}

void
CHPFCRFFSplitKernelAction::act()
{
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
  poly_params.applyParameters(parameters());
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
  poly_params.set<std::vector<VariableName>>("v") = real_v;
  _problem->addKernel("CHPFCRFF", "CH_bulk_n", poly_params);
}
