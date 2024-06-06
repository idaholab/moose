

#include "GAMMAmodelKernelAction.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"


registerMooseAction("PhaseFieldApp", GAMMAmodelKernelAction, "add_kernel");

InputParameters
GAMMAmodelKernelAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set up ACGrGrPoly, ACInterface, TimeDerivative, GAMMAmodelKERNELV2GAUSS, GAMMAmodelKERNELGAUSS kernels");
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
                        "The mobility is a function of any MOOSE variable (if "
                        "this is set to false, L must be constant over the "
                        "entire domain!)");
  params.addParam<std::vector<VariableName>>("args", "Vector of variable arguments L depends on");

  return params;

}

GAMMAmodelKernelAction::GAMMAmodelKernelAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),

    _var_name_base(getParam<std::string>("var_name_base"))

{
}

void
GAMMAmodelKernelAction::act()
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
    // Set up ACGrGrPoly kernels
    //

    {
      InputParameters params = _factory.getValidParams("ACGrGrPoly");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<VariableName>>("v") = v;
      params.applyParameters(parameters());

      std::string kernel_name = "ACBulk_" + var_name;
      _problem->addKernel("ACGrGrPoly", kernel_name, params);
    }

    //
    // Set up ACInterface kernels
    //

    {
      InputParameters params = _factory.getValidParams("ACInterface");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.applyParameters(parameters());

      std::string kernel_name = "ACInt_" + var_name;
      _problem->addKernel("ACInterface", kernel_name, params);
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


  }

//*******************************************************************************************************
//*******************************************************************************************************

     for (unsigned int m = 0; m < _op_num - 1; ++m)
     {

                  std::string var_name_plus  = _var_name_base + Moose::stringify(m);
                  std::vector<VariableName> vplus;
                  vplus.resize(_op_num - 1);
                  unsigned int indplus  = 0;
                  for (unsigned int l = 0; l < _op_num; ++l)
                    if (l != m)
                      vplus[indplus ++] = _var_name_base + Moose::stringify(m);
                      {
                        InputParameters params = _factory.getValidParams("GAMMAmodelKERNELV2GAUSS");
                        params.set<NonlinearVariableName>("variable") = var_name_plus;
                        params.set<std::vector<VariableName>>("vplus") = vplus;
                        params.applyParameters(parameters());

                        std::string kernel_name = "GAMMAmodelKERNELV2GAUSS" + var_name_plus;
                        _problem->addKernel("GAMMAmodelKERNELV2GAUSS", kernel_name, params);
                      }


                   std::string var_name_minus = _var_name_base + Moose::stringify(m+1);
                   std::vector<VariableName> vminus;
                   vminus.resize(_op_num - 1);
                   unsigned int indminus = 0;
                   for (unsigned int t = 0; t < _op_num; ++t)
                     if (t != (m+1))
                       vminus[indminus++] = _var_name_base + Moose::stringify(m+1);
                       {
                         InputParameters params = _factory.getValidParams("GAMMAmodelKERNELGAUSS");
                         params.set<NonlinearVariableName>("variable") = var_name_minus;
                         params.set<std::vector<VariableName>>("vminus") = vminus;
                         params.applyParameters(parameters());

                         std::string kernel_name = "GAMMAmodelKERNELGAUSS" + var_name_minus;
                         _problem->addKernel("GAMMAmodelKERNELGAUSS", kernel_name, params);
                       }
      }


}
