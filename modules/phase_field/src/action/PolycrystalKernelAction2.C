//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalKernelAction2.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"

registerMooseAction("PhaseFieldApp", PolycrystalKernelAction2, "add_kernel");

template <>
InputParameters
validParams<PolycrystalKernelAction2>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "Set up ADAllenCahn2, ADACInterface2, and TimeDerivative kernels");
  params.addRequiredParam<unsigned int>(
      "op_num", "specifies the total number of grains (deformed + recrystallized) to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<Real>("en_ratio", 1.0, "Ratio of surface to GB energy");
  params.addParam<unsigned int>("ndef", 0, "specifies the number of deformed grains to create");
  params.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<bool>("variable_mobility",
                        true,
                        "The mobility is a function of any MOOSE variable (if "
                        "this is set to false, L must be constant over the "
                        "entire domain!)");
  params.addParam<std::vector<VariableName>>("args", "Vector of variable arguments L depends on");
  params.addParam<MaterialPropertyName>("E_d",0,"Describes any extra energy in the system");
  return params;
}

PolycrystalKernelAction2::PolycrystalKernelAction2(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _E_d(getParam<MaterialPropertyName>("E_d")),
    _variable_mobility(getParam<bool>("variable_mobility"))
{
}

void
PolycrystalKernelAction2::act()
{
  for (unsigned int op = 0; op < _op_num; ++op)
  {
    //
    // Create variable names
    //

    std::string var_name = _var_name_base + Moose::stringify(op);
    std::vector<VariableName> v;
    v.resize(_op_num - 1);

    unsigned int ind = 0;
    for (unsigned int j = 0; j < _op_num; ++j)
      if (j != op)
        v[ind++] = _var_name_base + Moose::stringify(j);

    //
    // Set up ADAllenCahn2 kernels
    //

    {
      InputParameters params = _factory.getValidParams("AllenCahn");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<MaterialPropertyName>("f_name") = "kappa_op";
//      params.set<bool>("extra_term") = 1;
      params.applyParameters(parameters());

      std::string kernel_name = "AllenCahn_F_gr" + var_name;
      _problem->addKernel("AllenCahn", kernel_name, params);
    }
    {
      InputParameters params = _factory.getValidParams("ADAllenCahn2");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<MaterialPropertyName>("f_name") = "Moelans_F_loc";
      params.applyParameters(parameters());

      std::string kernel_name = "AllenCahn_F_loc" + var_name;
      _problem->addKernel("ADAllenCahn2", kernel_name, params);
    }
    {
      InputParameters params = _factory.getValidParams("ADAllenCahn2");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<MaterialPropertyName>("f_name") = _E_d;
      params.applyParameters(parameters());

      std::string kernel_name = "AllenCahn_F_loc" + var_name;
      _problem->addKernel("ADAllenCahn2", kernel_name, params);
    }

    //
    // Set up ACInterface2 kernels
    //

    {
      InputParameters params = _factory.getValidParams("ACInterface2");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<bool>("variable_L") = _variable_mobility;
      params.applyParameters(parameters());

      std::string kernel_name = "ACInt_" + var_name;
      _problem->addKernel("ACInterface2", kernel_name, params);
    }
    {
      InputParameters params = _factory.getValidParams("ACInterface2");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<bool>("variable_L") = _variable_mobility;
      params.set<bool>("extra_term") = 1;
      params.applyParameters(parameters());

      std::string kernel_name = "ACInt_kappa_" + var_name;
      _problem->addKernel("ACInterface2", kernel_name, params);
    }

    //
    // Set up TimeDerivative kernels
    //

    {
      InputParameters params = _factory.getValidParams("TimeDerivative");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<bool>("implicit") = true;
      params.applyParameters(parameters());

      std::string kernel_name = "IE_" + var_name;
      _problem->addKernel("TimeDerivative", kernel_name, params);
    }

    //
    // Set up optional ACGBPoly bubble interaction kernels
    //

  }
}
