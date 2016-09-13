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
  InputParameters params = validParams<Action>();
  params.addClassDescription("Action for applying AllenCahn equations and SingleGrainRigidBodyMotion to grains");
  params.addRequiredParam<unsigned int>("op_num", "specifies the number of grains to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<VariableName>("c", "Name of coupled concentration variable");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define type of force density under consideration");
  params.addParam<Real>("translation_constant", 500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  params.addParam<UserObjectName>("grain_force", "userobject for getting force and torque acting on grains");
  params.addParam<UserObjectName>("grain_tracker_object", "The FeatureFloodCount UserObject to get values from.");
  params.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

RigidBodyMultiKernelAction::RigidBodyMultiKernelAction(const InputParameters & params) :
  Action(params),
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
      InputParameters params = _factory.getValidParams("ACInterface");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<bool>("implicit") = getParam<bool>("implicit");
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      params.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa_name");
      params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mob_name");

      std::string kernel_name = "ACInt_" + var_name;
      _problem->addKernel("ACInterface", kernel_name, params);
    }

    //
    // Set up the AllenCahn kernels
    //

    {
      InputParameters params = _factory.getValidParams("AllenCahn");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<VariableName> >("args") = v;
      params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mob_name");
      params.set<MaterialPropertyName>("f_name") = getParam<MaterialPropertyName>("f_name");
      params.set<bool>("implicit") = _implicit;
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      std::string kernel_name = "AC_" + var_name;
      _problem->addKernel("AllenCahn", kernel_name, params);
    }

    //
    // Set up SingleGrainRigidBodyMotion kernels
    //

    {
      InputParameters params = _factory.getValidParams("SingleGrainRigidBodyMotion");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<VariableName> >("v") = v;
      params.set<unsigned int>("op_index") = op;
      params.set<std::vector<VariableName> >("c") = {getParam<VariableName>("c")};
      if (isParamValid("base_name"))
        params.set<std::string>("base_name") = getParam<std::string>("base_name");
      params.set<Real>("translation_constant") = getParam<Real>("translation_constant");
      params.set<Real>("rotation_constant") = getParam<Real>("rotation_constant");
      params.set<UserObjectName>("grain_force") = getParam<UserObjectName>("grain_force");
      params.set<UserObjectName>("grain_tracker_object") = getParam<UserObjectName>("grain_tracker_object");

      params.set<bool>("implicit") = _implicit;
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      std::string kernel_name = "RigidBody_" + var_name;
      _problem->addKernel("SingleGrainRigidBodyMotion", kernel_name, params);
    }

    //
    // Set up TimeDerivative kernels
    //

    {
      InputParameters params = _factory.getValidParams("TimeDerivative");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<bool>("implicit") = true;
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      std::string kernel_name = "IE_" + var_name;
      _problem->addKernel("TimeDerivative", kernel_name, params);
    }
  }
}
