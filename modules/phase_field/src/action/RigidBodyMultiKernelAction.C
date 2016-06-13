/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RigidBodyMultiKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "Conversion.h"
#include "FEProblem.h"

template<>
InputParameters validParams<RigidBodyMultiKernelAction>()
{
  InputParameters parameters = validParams<Action>();

  parameters.addRequiredParam<unsigned int>("op_num", "specifies the number of grains to create");
  parameters.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  parameters.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  parameters.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  parameters.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  parameters.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  parameters.addParam<MaterialPropertyName>("f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  parameters.addParam<VariableName>("c", "Name of coupled concentration variable");

  return parameters;
}

RigidBodyMultiKernelAction::RigidBodyMultiKernelAction(const InputParameters & parameters) :
  Action(parameters),
  _op_num(getParam<unsigned int>("op_num")),
  _var_name_base(getParam<std::string>("var_name_base")),
  _implicit(getParam<bool>("implicit"))
{
}

void
RigidBodyMultiKernelAction::act()
{
  for (unsigned int op = 0; op < _op_num; ++op)
  {
    //
    // Create variable names
    //

    std::string var_name = _var_name_base + Moose::stringify(op);
    std::vector<VariableName> v;
    unsigned int ind = 0;

    if (isParamValid("c"))
    {
      VariableName c = getParam<VariableName>("c");
      v.resize(_op_num);

      for (unsigned int j = 0; j < _op_num; ++j)
        if (j != op)
          v[ind++] = _var_name_base + Moose::stringify(j);

        v[ind++] = c;
    }
    else
    {
      v.resize(_op_num - 1);
      for (unsigned int j = 0; j < _op_num; ++j)
        if (j != op)
          v[ind++] = _var_name_base + Moose::stringify(j);
    }

    //
    // Set up ACInterface kernels
    //

    {
      InputParameters parameters = _factory.getValidParams("ACInterface");
      parameters.set<NonlinearVariableName>("variable") = var_name;
      parameters.set<bool>("implicit") = getParam<bool>("implicit");
      parameters.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      parameters.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa_name");
      parameters.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mob_name");

      std::string kernel_name = "ACInt_" + var_name;
      _problem->addKernel("ACInterface", kernel_name, parameters);
    }

    //
    // Set up the AllenCahn kernels
    //

    {
      InputParameters parameters = _factory.getValidParams("AllenCahn");
      parameters.set<NonlinearVariableName>("variable") = var_name;
      parameters.set<std::vector<VariableName> >("args") = v;
      parameters.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mob_name");
      parameters.set<MaterialPropertyName>("f_name") = getParam<MaterialPropertyName>("f_name");
      parameters.set<bool>("implicit") = _implicit;
      parameters.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      std::string kernel_name = "AC_" + var_name;
      _problem->addKernel("AllenCahn", kernel_name, parameters);
    }

    //
    // Set up SingleGrainRigidBodyMotion kernels
    //

    {
      InputParameters parameters = _factory.getValidParams("SingleGrainRigidBodyMotion");
      parameters.set<NonlinearVariableName>("variable") = var_name;
      parameters.set<std::vector<VariableName> >("v") = v;
      parameters.set<std::vector<VariableName> >("c") = {getParam<VariableName>("c")};
      parameters.set<bool>("implicit") = _implicit;
      parameters.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      std::string kernel_name = "RigidBody_" + var_name;
      _problem->addKernel("SingleGrainRigidBodyMotion", kernel_name, parameters);
    }

    //
    // Set up TimeDerivative kernels
    //

    {
      InputParameters parameters = _factory.getValidParams("TimeDerivative");
      parameters.set<NonlinearVariableName>("variable") = var_name;
      parameters.set<bool>("implicit") = true;
      parameters.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      std::string kernel_name = "IE_" + var_name;
      _problem->addKernel("TimeDerivative", kernel_name, parameters);
    }
  }
}
