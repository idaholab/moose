//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFCRFFKernelAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("PhaseFieldApp", PFCRFFKernelAction, "add_kernel");

InputParameters
PFCRFFKernelAction::validParams()
{
  InputParameters params = HHPFCRFFSplitKernelAction::validParams();
  params.addParam<Real>("a", 1.0, "Parameter in the Taylor series expansion");
  params.addParam<Real>("b", 1.0, "Parameter in the Taylor series expansion");
  params.addParam<Real>("c", 1.0, "Parameter in the Taylor series expansion");
  return params;
}

PFCRFFKernelAction::PFCRFFKernelAction(const InputParameters & params)
  : HHPFCRFFSplitKernelAction(params)
{
}

void
PFCRFFKernelAction::act()
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
  poly_params.set<NonlinearVariableName>("variable") = _n_name;
  poly_params.set<std::vector<VariableName>>("v") = real_v;
  poly_params.applyParameters(parameters());
  _problem->addKernel("CHPFCRFF", "CH_bulk_n", poly_params);

  // Loop over the L_variables
  HHPFCRFFSplitKernelAction::act();
}
