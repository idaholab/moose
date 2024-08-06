#include "EpsilonModelKernelAction.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"

registerMooseAction("PhaseFieldApp", EpsilonModelKernelAction, "add_kernel");

InputParameters
EpsilonModelKernelAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up ACGrGrPoly, ACInterface, TimeDerivative, ACGBPoly, "
                             "EpsilonModelKernel1stGauss, EpsilonModelKernel2ndGauss, "
                             "EpsilonModelKernel1stV2Gauss, EpsilonModelKernel2ndV2Gauss kernels");
  params.addRequiredParam<unsigned int>(
      "op_num", "specifies the total number of grains (deformed + recrystallized) to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<VariableName>("c", "Name of coupled concentration variable");
  params.addParam<Real>("en_ratio", 1.0, "Ratio of surface to GB energy");
  params.addParam<unsigned int>("ndef", 0, "specifies the number of deformed grains to create");
  params.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<bool>("variable_mobility",
                        true,
                        "The mobility is a function of any MOOSE variable (if this is set to "
                        "false, L must be constant over the entire domain!)");
  params.addParam<std::vector<VariableName>>("args", "Vector of variable arguments L depends on");
  return params;
}

EpsilonModelKernelAction::EpsilonModelKernelAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
EpsilonModelKernelAction::act()
{
  for (unsigned int op = 0; op < _op_num; ++op)
  {
    std::string var_name = _var_name_base + Moose::stringify(op);
    std::vector<VariableName> v(_op_num - 1);
    unsigned int ind = 0;
    for (unsigned int j = 0; j < _op_num; ++j)
      if (j != op)
        v[ind++] = _var_name_base + Moose::stringify(j);

    // Set up ACGrGrPoly kernel
    InputParameters params = _factory.getValidParams("ACGrGrPoly");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<std::vector<VariableName>>("v") = v;
    params.applyParameters(parameters());
    _problem->addKernel("ACGrGrPoly", "ACBulk_" + var_name, params);

    // Set up ACInterface kernel
    params = _factory.getValidParams("ACInterface");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.applyParameters(parameters());
    _problem->addKernel("ACInterface", "ACInt_" + var_name, params);

    // Set up TimeDerivative kernel
    params = _factory.getValidParams("TimeDerivative");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<bool>("implicit") = true;
    params.applyParameters(parameters());
    _problem->addKernel("TimeDerivative", "IE_" + var_name, params);

    // Set up EpsilonModelKernel1stGauss and EpsilonModelKernel2ndGauss kernels
    if (op < _op_num - 1)
    {
      std::string var_name_plus = _var_name_base + Moose::stringify(op);
      params = _factory.getValidParams("EpsilonModelKernel1stGauss");
      params.set<NonlinearVariableName>("variable") = var_name_plus;
      params.set<std::vector<VariableName>>("v") = v;
      params.applyParameters(parameters());
      _problem->addKernel(
          "EpsilonModelKernel1stGauss", "EpsilonModelKernel1stGauss" + var_name_plus, params);

      params = _factory.getValidParams("EpsilonModelKernel2ndGauss");
      params.set<NonlinearVariableName>("variable") = var_name_plus;
      params.applyParameters(parameters());
      _problem->addKernel(
          "EpsilonModelKernel2ndGauss", "EpsilonModelKernel2ndGauss" + var_name_plus, params);
    }

    // Set up EpsilonModelKernel1stV2Gauss and EpsilonModelKernel2ndV2Gauss kernels
    if (op > 0)
    {
      std::string var_name_minus = _var_name_base + Moose::stringify(op);
      params = _factory.getValidParams("EpsilonModelKernel1stV2Gauss");
      params.set<NonlinearVariableName>("variable") = var_name_minus;
      params.set<std::vector<VariableName>>("v") = v;
      params.applyParameters(parameters());
      _problem->addKernel(
          "EpsilonModelKernel1stV2Gauss", "EpsilonModelKernel1stV2Gauss" + var_name_minus, params);

      params = _factory.getValidParams("EpsilonModelKernel2ndV2Gauss");
      params.set<NonlinearVariableName>("variable") = var_name_minus;
      params.applyParameters(parameters());
      _problem->addKernel(
          "EpsilonModelKernel2ndV2Gauss", "EpsilonModelKernel2ndV2Gauss" + var_name_minus, params);
    }
  }
}
