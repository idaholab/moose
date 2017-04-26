/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PolycrystalKernelAction.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<PolycrystalKernelAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "Set up ACGrGrPoly, ACInterface, TimeDerivative, and ACGBPoly kernels");
  params.addRequiredParam<unsigned int>(
      "op_num", "specifies the total number of grains/variants (in all phases) to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<VariableName>("c", "Name of coupled concentration variable");
  params.addParam<Real>("en_ratio", 1.0, "Ratio of surface to GB energy");
  params.addParam<unsigned int>("ndef", 0, "specifies the number of deformed grains to create");
  params.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  params.addParam<VariableName>("T", "Name of temperature variable");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

PolycrystalKernelAction::PolycrystalKernelAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalKernelAction::act()
{
  for (_op = 0; _op < _op_num; ++_op)
  {
    //
    // Create variable names
    //

    std::string var_name = _var_name_base + Moose::stringify(_op);
    std::vector<VariableName> v;
    v.resize(_op_num - 1);

    unsigned int ind = 0;
    for (unsigned int j = 0; j < _op_num; ++j)
      if (j != _op)
        v[ind++] = _var_name_base + Moose::stringify(j);

    //
    // Set up ACGrGrPoly (for single phase) or ACTwoPhaseGrGrPoly (for two-phase) kernels
    //

    {
      InputParameters params = _factory.getValidParams(getACBulkName());
      // params.applyParameters(parameters());

      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<VariableName>>("v") = v;
      if (isParamValid("T"))
        params.set<std::vector<VariableName>>("T") = {getParam<VariableName>("T")};
      params.applyParameters(parameters());

      setupACBulkKernel(params, var_name);
    }

    //
    // Set up ACInterface kernels
    //

    {
      InputParameters params = _factory.getValidParams("ACInterface");

      params.set<NonlinearVariableName>("variable") = var_name;
      params.applyParameters(parameters());

      _problem->addKernel("ACInterface", "ACInt_" + var_name, params);
    }

    //
    // Set up TimeDerivative kernels
    //

    {
      InputParameters params = _factory.getValidParams("TimeDerivative");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.applyParameters(parameters());
      params.set<bool>("implicit") = true;

      _problem->addKernel("TimeDerivative", "IE_" + var_name, params);
    }

    //
    // Set up optional ACGBPoly bubble interaction kernels
    //

    if (isParamValid("c"))
    {
      InputParameters params = _factory.getValidParams("ACGBPoly");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<VariableName>>("c") = {getParam<VariableName>("c")};
      params.applyParameters(parameters());

      _problem->addKernel("ACGBPoly", "ACBubInteraction_" + var_name, params);
    }
  }
}

void
PolycrystalKernelAction::setupACBulkKernel(InputParameters params, std::string var_name)
{
  _problem->addKernel(getACBulkName(), "ACBulk_" + var_name, params);
}
