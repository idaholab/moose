/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MatVecRealGradAuxKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "Conversion.h"
#include "FEProblem.h"

template<>
InputParameters validParams<MatVecRealGradAuxKernelAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<std::vector<unsigned int> >("op_num", "Vector that specifies the number of grains to create");
  params.addRequiredParam<std::vector<std::string> >("var_name_base", "Vector specifies the base name of the variables");
  params.addRequiredParam<MaterialPropertyName>("property", "the scalar material property name");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");

  return params;
}

MatVecRealGradAuxKernelAction::MatVecRealGradAuxKernelAction(const InputParameters & params) :
    Action(params)
{
}

void
MatVecRealGradAuxKernelAction::act()
{
  std::vector<unsigned int> _op_num = getParam<std::vector<unsigned int> >("op_num");
  std::vector<std::string> _var_name_base = getParam<std::vector<std::string> >("var_name_base");
  unsigned int size_o = _op_num.size();
  unsigned int size_v = _var_name_base.size();

  if (size_o != size_v)
    mooseError("op_num and var_name_base must be vectors of the same dimension");
  for (unsigned int val = 0; val < size_o; val++)
  {
    for (unsigned int op = 0; op < _op_num[val]; ++op)
    {
      //
      // Create variable names
      //

      std::string var_name = _var_name_base[val] + Moose::stringify(op);

      //
      // Set up MaterialStdVectorRealGradientAux auxkernel
      //

      {
        InputParameters params = _factory.getValidParams("MaterialStdVectorRealGradientAux");
        params.set<AuxVariableName>("variable") = var_name;
        params.set<MaterialPropertyName>("property") = getParam<MaterialPropertyName>("property");
        params.set<unsigned int>("component") = val;
        params.set<unsigned int>("index") = op;
        params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

        std::string aux_kernel_name = "grad_" + var_name;
        _problem->addAuxKernel("MaterialStdVectorRealGradientAux", aux_kernel_name, params);
      }
    }
  }
}
