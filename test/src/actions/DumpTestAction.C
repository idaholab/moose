//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DumpTestAction.h"
#include "FEProblem.h"

registerMooseAction("MooseTestApp", DumpTestAction, "add_aux_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_aux_scalar_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_aux_variable");
registerMooseAction("MooseTestApp", DumpTestAction, "add_bc");
registerMooseAction("MooseTestApp", DumpTestAction, "add_constraint");
registerMooseAction("MooseTestApp", DumpTestAction, "add_damper");
registerMooseAction("MooseTestApp", DumpTestAction, "add_dg_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_dirac_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_distribution");
registerMooseAction("MooseTestApp", DumpTestAction, "add_function");
registerMooseAction("MooseTestApp", DumpTestAction, "add_fv_bc");
registerMooseAction("MooseTestApp", DumpTestAction, "add_fv_ik");
registerMooseAction("MooseTestApp", DumpTestAction, "add_fv_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_indicator");
registerMooseAction("MooseTestApp", DumpTestAction, "add_ic");
registerMooseAction("MooseTestApp", DumpTestAction, "add_interface_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_marker");
registerMooseAction("MooseTestApp", DumpTestAction, "add_material");
registerMooseAction("MooseTestApp", DumpTestAction, "add_multi_app");
registerMooseAction("MooseTestApp", DumpTestAction, "add_nodal_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "add_postprocessor");
registerMooseAction("MooseTestApp", DumpTestAction, "setup_predictor");
registerMooseAction("MooseTestApp", DumpTestAction, "add_reporter");
registerMooseAction("MooseTestApp", DumpTestAction, "add_sampler");
registerMooseAction("MooseTestApp", DumpTestAction, "add_scalar_kernel");
registerMooseAction("MooseTestApp", DumpTestAction, "setup_time_integrator");
registerMooseAction("MooseTestApp", DumpTestAction, "add_transfer");
registerMooseAction("MooseTestApp", DumpTestAction, "add_user_object");
registerMooseAction("MooseTestApp", DumpTestAction, "add_variable");
registerMooseAction("MooseTestApp", DumpTestAction, "add_vector_postprocessor");

InputParameters
DumpTestAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

DumpTestAction::DumpTestAction(const InputParameters & parameters) : Action(parameters) {}

void
DumpTestAction::act()
{
  if (_current_task == "add_aux_kernel")
  {
    auto params = _factory.getValidParams("ConstantAux");
    params.set<AuxVariableName>("variable") = {"u"};
    params.set<Real>("value") = {42};
    _problem->addAuxKernel("ConstantAux", "aux_kernel", params);
  }
  else if (_current_task == "add_aux_scalar_kernel")
  {
    auto params = _factory.getValidParams("ConstantScalarAux");
    params.set<AuxVariableName>("variable") = {"q"};
    params.set<Real>("value") = {9.9};
    _problem->addAuxKernel("ConstantScalarAux", "aux_scalar_kernel", params);
  }
  else if (_current_task == "add_aux_variable")
  {
    auto var_params = _factory.getValidParams("MooseVariable");
    var_params.set<MooseEnum>("order") = "FIRST";
    var_params.set<MooseEnum>("family") = "LAGRANGE";
    _problem->addAuxVariable("MooseVariable", "u", var_params);
  }
  else if (_current_task == "add_bc")
  {
    auto params = _factory.getValidParams("DirichletBC");
    params.set<NonlinearVariableName>("variable") = {"v"};
    params.set<Real>("value") = {0};
    params.set<std::vector<BoundaryName>>("boundary") = {"left"};
    _problem->addBoundaryCondition("DirichletBC", "bc", params);
  }
  else if (_current_task == "add_constraint")
  {
    auto params = _factory.getValidParams("CoupledTiedValueConstraint");
    params.set<NonlinearVariableName>("variable") = {"v"};
    params.set<std::vector<VariableName>>("primary_variable") = {"u"};
    params.set<BoundaryName>("secondary") = {"right"};
    params.set<BoundaryName>("primary") = {"top"};
    _problem->addConstraint("CoupledTiedValueConstraint", "constraint", params);
  }
  else if (_current_task == "add_damper")
  {
    auto params = _factory.getValidParams("ConstantDamper");
    params.set<Real>("damping") = 0.1;
    _problem->addDamper("ConstantDamper", "damper", params);
  }
  else if (_current_task == "add_dg_kernel")
  {
    auto params = _factory.getValidParams("DGDiffusion");
    params.set<NonlinearVariableName>("variable") = "v";
    params.set<Real>("sigma") = 0.1;
    params.set<Real>("epsilon") = 0.2;
    _problem->addDGKernel("DGDiffusion", "dg_kernel", params);
  }
  else if (_current_task == "add_dirac_kernel")
  {
    auto params = _factory.getValidParams("ConstantPointSource");
    params.set<NonlinearVariableName>("variable") = "v";
    params.set<Real>("value") = 0.3;
    params.set<std::vector<Real>>("point") = {1.1, 2.2, 3.3};
    _problem->addDiracKernel("ConstantPointSource", "dirackernel", params);
  }
  else if (_current_task == "add_distribution")
  {
    // no working distribution exists in framework
  }
  else if (_current_task == "add_function")
  {
    auto params = _factory.getValidParams("ConstantFunction");
    params.set<Real>("value") = 0.44;
    _problem->addFunction("ConstantFunction", "function", params);
  }
  else if (_current_task == "add_fv_bc")
  {
    auto params = _factory.getValidParams("FVDirichletBC");
    params.set<NonlinearVariableName>("variable") = "f";
    params.set<Real>("value") = 0.5;
    params.set<std::vector<BoundaryName>>("boundary") = {"right"};
    _problem->addFVBC("FVDirichletBC", "fvbc", params);
  }
  else if (_current_task == "add_fv_ik")
  {
    // auto params = _factory.getValidParams("FVDiffusionInterface");
    // params.set<NonlinearVariableName>("variable") = "f";
    // params.set<MaterialPropertyName>("coeff1") = "0.7";
    // params.set<MaterialPropertyName>("coeff2") = "0.8";
    // _problem->addFVInterfaceKernel("FVDiffusionInterface", "fvik", params);
  }
  else if (_current_task == "add_fv_kernel")
  {
    auto params = _factory.getValidParams("FVDiffusion");
    params.set<NonlinearVariableName>("variable") = "f";
    params.set<MooseFunctorName>("coeff") = "0.6";
    _problem->addFVKernel("FVDiffusion", "fvkernel", params);
  }
  else if (_current_task == "add_indicator")
  {
    auto params = _factory.getValidParams("ValueJumpIndicator");
    params.set<VariableName>("variable") = "v";
    _problem->addIndicator("ValueJumpIndicator", "indicator", params);
  }
  else if (_current_task == "add_ic")
  {
    auto params = _factory.getValidParams("ConstantIC");
    params.set<VariableName>("variable") = "u";
    params.set<Real>("value") = 0.9;
    _problem->addInitialCondition("ConstantIC", "ic", params);
  }
  else if (_current_task == "add_interface_kernel")
  {
  }
  else if (_current_task == "add_kernel")
  {
    auto params = _factory.getValidParams("Diffusion");
    params.set<NonlinearVariableName>("variable") = "v";
    _problem->addKernel("Diffusion", "kernel", params);
  }
  else if (_current_task == "add_marker")
  {
  }
  else if (_current_task == "add_material")
  {
  }
  else if (_current_task == "add_multi_app")
  {
  }
  else if (_current_task == "add_nodal_kernel")
  {
  }
  else if (_current_task == "add_postprocessor")
  {
  }
  else if (_current_task == "setup_predictor")
  {
  }
  else if (_current_task == "add_reporter")
  {
  }
  else if (_current_task == "add_sampler")
  {
  }
  else if (_current_task == "add_scalar_kernel")
  {
  }
  else if (_current_task == "setup_time_integrator")
  {
  }
  else if (_current_task == "add_transfer")
  {
  }
  else if (_current_task == "add_user_object")
  {
  }
  else if (_current_task == "add_variable")
  {
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = "FIRST";
      var_params.set<MooseEnum>("family") = "LAGRANGE";
      _problem->addVariable("MooseVariable", "v", var_params);
    }
    {
      auto var_params = _factory.getValidParams("MooseVariableFVReal");
      _problem->addVariable("MooseVariableFVReal", "f", var_params);
    }
    {
      auto var_params = _factory.getValidParams("MooseVariableConstMonomial");
      var_params.set<MooseEnum>("order") = "CONSTANT";
      var_params.set<MooseEnum>("family") = "MONOMIAL";
      _problem->addVariable("MooseVariableConstMonomial", "indicator", var_params);
    }
  }
  else if (_current_task == "add_vector_postprocessor")
  {
  }
  else
    mooseError("Forgot to handle '", _current_task, "'");
}
