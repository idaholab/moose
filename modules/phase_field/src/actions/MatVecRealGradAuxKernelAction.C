//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatVecRealGradAuxKernelAction.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"

registerMooseAction("PhaseFieldApp", MatVecRealGradAuxKernelAction, "add_aux_kernel");

InputParameters
MatVecRealGradAuxKernelAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Outputs all components of the gradient of the real standard "
                             "vector-valued properties specified");
  params.addRequiredParam<unsigned int>("op_num",
                                        "Value that specifies the number of grains to create");
  params.addRequiredParam<std::vector<std::string>>(
      "var_name_base", "Vector specifies the base name of the variables");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("property",
                                                             "the scalar material property names");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addRequiredParam<unsigned int>("dim", "the dimensions of the mesh");
  params.addParam<AuxVariableName>("divergence_variable",
                                   "Name of divergence variable to generate kernels for");
  params.addParam<MaterialPropertyName>("divergence_property",
                                        "Scalar material property name for divergence variable");
  return params;
}

MatVecRealGradAuxKernelAction::MatVecRealGradAuxKernelAction(const InputParameters & params)
  : Action(params),
    _div_var(getParam<AuxVariableName>("divergence_variable")),
    _prop(getParam<std::vector<MaterialPropertyName>>("property")),
    _div_prop(getParam<MaterialPropertyName>("divergence_property"))
{
  mooseDeprecated("Use 'MaterialVectorAuxKernel' or 'MaterialVectorGradAuxKernel' action instead "
                  "depending on data_type of MaterialProperty<std::vector<data_type> >");
}

void
MatVecRealGradAuxKernelAction::act()
{
  const std::vector<std::string> var_name_base =
      getParam<std::vector<std::string>>("var_name_base");

  const unsigned int op_num = getParam<unsigned int>("op_num");
  const unsigned int dim = getParam<unsigned int>("dim");
  const unsigned int size_v = var_name_base.size();
  const unsigned int size_p = _prop.size();

  if (size_p != size_v)
    paramError("property", "var_name_base and property must be vectors of the same dimension");

  for (unsigned int op = 0; op < op_num; ++op)
  {
    for (unsigned int val = 0; val < size_v; ++val)
      for (unsigned int x = 0; x < dim; ++x)
      {
        std::string var_name = var_name_base[val] + Moose::stringify(x) + Moose::stringify(op);
        {
          InputParameters params = _factory.getValidParams("MaterialStdVectorRealGradientAux");
          params.set<AuxVariableName>("variable") = var_name;
          params.set<MaterialPropertyName>("property") = _prop[val];
          params.set<unsigned int>("component") = x;
          params.set<unsigned int>("index") = op;
          params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
          _problem->addAuxKernel("MaterialStdVectorRealGradientAux", "grad_" + var_name, params);
        }
      }

    if (isParamValid("divergence_variable"))
    {
      if (isParamValid("divergence_property"))
      {
        InputParameters params = _factory.getValidParams("MaterialStdVectorAux");
        params.set<AuxVariableName>("variable") = _div_var;
        params.set<MaterialPropertyName>("property") = _div_prop;
        params.set<unsigned int>("index") = op;
        params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
        _problem->addAuxKernel("MaterialStdVectorAux", "div_" + Moose::stringify(op), params);
      }
      else
        mooseError("Must specify a divergence_property name along with divergence_variable name");
    }
  }
}
