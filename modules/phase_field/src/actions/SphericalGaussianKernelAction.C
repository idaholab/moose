//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SphericalGaussianKernelAction.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"

registerMooseAction("PhaseFieldApp", SphericalGaussianKernelAction, "add_kernel");

InputParameters
SphericalGaussianKernelAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up ADGrainGrowth, ADACInterface, ADTimeDerivative, "
                             "EpsilonModelEpsilonGradientKernel, EpsilonModelMGradientKernel, "
                             "GammaModelGammaGradientKernel kernels based on the selected model");
  MooseEnum models("EPSILON GAMMA");
  params.addRequiredParam<MooseEnum>("model_type", models, "Epsilon or Gamma model?");
  params.addRequiredParam<unsigned int>(
      "op_num", "Specifies the total number of coupled order parameter variables");
  params.addRequiredParam<std::string>("var_name_base", "Specifies the base name of the variables");
  return params;
}

SphericalGaussianKernelAction::SphericalGaussianKernelAction(const InputParameters & params)
  : Action(params),
    _model_type(getParam<MooseEnum>("model_type").getEnum<ModelType>()),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))

{
}

void
SphericalGaussianKernelAction::act()
{
  for (unsigned int op = 0; op < _op_num; ++op)
  {
    std::string var_name = _var_name_base + Moose::stringify(op);
    std::vector<VariableName> v(_op_num - 1);
    unsigned int ind = 0;
    for (unsigned int j = 0; j < _op_num; ++j)
      if (j != op)
        v[ind++] = _var_name_base + Moose::stringify(j);

    // Set up ADGrainGrowth kernel
    InputParameters params = _factory.getValidParams("ADGrainGrowth");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<std::vector<VariableName>>("v") = v;
    params.applyParameters(parameters());
    _problem->addKernel("ADGrainGrowth", "ADGrainGrowth_" + var_name, params);

    // Set up ADACInterface kernel
    params = _factory.getValidParams("ADACInterface");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<bool>("variable_L") = false;
    params.applyParameters(parameters());
    _problem->addKernel("ADACInterface", "ADACInterface_" + var_name, params);

    // Set up ADTimeDerivative kernel
    params = _factory.getValidParams("ADTimeDerivative");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<bool>("implicit") = true;
    params.applyParameters(parameters());
    _problem->addKernel("ADTimeDerivative", "ADTimeDerivative_" + var_name, params);

    if (_model_type == ModelType::EPSILON) // If model_type is "EPSILON"
    {
      // Set up EpsilonModelEpsilonGradientKernel and EpsilonModelMGradientKernel kernels for the
      // first set of grains
      if (op < _op_num - 1)
      {
        std::string var_name_plus = _var_name_base + Moose::stringify(op);
        params = _factory.getValidParams("EpsilonModelEpsilonGradientKernel");
        params.set<NonlinearVariableName>("variable") = var_name_plus;
        params.set<std::vector<VariableName>>("v") = v;
        params.set<MooseEnum>("grains_set") = "FIRST";
        params.applyParameters(parameters());
        _problem->addKernel("EpsilonModelEpsilonGradientKernel",
                            "EpsilonModelEpsilonGradientKernel_plus" + var_name_plus,
                            params);

        params = _factory.getValidParams("EpsilonModelMGradientKernel");
        params.set<NonlinearVariableName>("variable") = var_name_plus;
        params.set<MooseEnum>("grains_set") = "FIRST";
        params.applyParameters(parameters());
        _problem->addKernel("EpsilonModelMGradientKernel",
                            "EpsilonModelMGradientKernel_plus" + var_name_plus,
                            params);
      }

      // Set up EpsilonModelEpsilonGradientKernel and EpsilonModelMGradientKernel kernels for the
      // second set of grains
      if (op > 0)
      {
        std::string var_name_minus = _var_name_base + Moose::stringify(op);
        params = _factory.getValidParams("EpsilonModelEpsilonGradientKernel");
        params.set<NonlinearVariableName>("variable") = var_name_minus;
        params.set<std::vector<VariableName>>("v") = v;
        params.set<MooseEnum>("grains_set") = "SECOND";
        params.applyParameters(parameters());
        _problem->addKernel("EpsilonModelEpsilonGradientKernel",
                            "EpsilonModelEpsilonGradientKernel_minus" + var_name_minus,
                            params);

        params = _factory.getValidParams("EpsilonModelMGradientKernel");
        params.set<NonlinearVariableName>("variable") = var_name_minus;
        params.set<MooseEnum>("grains_set") = "SECOND";
        params.applyParameters(parameters());
        _problem->addKernel("EpsilonModelMGradientKernel",
                            "EpsilonModelMGradientKernel_minus" + var_name_minus,
                            params);
      }
    }
    else if (_model_type == ModelType::GAMMA) // If model_type is "GAMMA"
    {
      // Set up GammaModelGammaGradientKernel kernel for the first set of grains
      if (op < _op_num - 1)
      {
        std::string var_name_plus = _var_name_base + Moose::stringify(op);
        params = _factory.getValidParams("GammaModelGammaGradientKernel");
        params.set<NonlinearVariableName>("variable") = var_name_plus;
        params.set<std::vector<VariableName>>("v") = v;
        params.set<MooseEnum>("grains_set") = "FIRST";
        params.applyParameters(parameters());
        _problem->addKernel("GammaModelGammaGradientKernel",
                            "GammaModelGammaGradientKernel_plus" + var_name_plus,
                            params);
      }

      // Set up GammaModelGammaGradientKernel kernel for the second set of grains
      if (op > 0)
      {
        std::string var_name_minus = _var_name_base + Moose::stringify(op);
        params = _factory.getValidParams("GammaModelGammaGradientKernel");
        params.set<NonlinearVariableName>("variable") = var_name_minus;
        params.set<std::vector<VariableName>>("v") = v;
        params.set<MooseEnum>("grains_set") = "SECOND";
        params.applyParameters(parameters());
        _problem->addKernel("GammaModelGammaGradientKernel",
                            "GammaModelGammaGradientKernel_minus" + var_name_minus,
                            params);
      }
    }
    else
    {
      mooseError("Invalid model_type value"); // Handle unexpected values of _model_type
    }
  }
}
