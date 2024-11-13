//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialVectorAuxKernelAction.h"
#include "Factory.h"
#include "Conversion.h"
#include "FEProblem.h"

registerMooseAction("PhaseFieldApp", MaterialVectorAuxKernelAction, "add_aux_kernel");

InputParameters
MaterialVectorAuxKernelAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Outputs all components of the standard vector-valued properties specified");
  params.addRequiredParam<unsigned int>(
      "grain_num", "Value that specifies the number of grains to create aux kernels for.");
  params.addRequiredParam<std::vector<std::string>>(
      "variable_base", "Vector specifies the base name of the variables.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("property",
                                                             "The material property names.");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels.");
  return params;
}

MaterialVectorAuxKernelAction::MaterialVectorAuxKernelAction(const InputParameters & params)
  : Action(params),
    _grain_num(getParam<unsigned int>("grain_num")),
    _var_name_base(getParam<std::vector<std::string>>("variable_base")),
    _num_var(_var_name_base.size()),
    _prop(getParam<std::vector<MaterialPropertyName>>("property")),
    _num_prop(_prop.size())
{
}

void
MaterialVectorAuxKernelAction::act()
{
  if (_num_prop != _num_var)
    paramError("property", "variable_base and property must be vectors of the same size");

  for (unsigned int gr = 0; gr < _grain_num; ++gr)
    for (unsigned int val = 0; val < _num_var; ++val)
    {
      std::string var_name = _var_name_base[val] + Moose::stringify(gr);

      InputParameters params = _factory.getValidParams("MaterialStdVectorAux");
      params.set<AuxVariableName>("variable") = var_name;
      params.set<MaterialPropertyName>("property") = _prop[val];
      params.set<unsigned int>("index") = gr;
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

      std::string aux_kernel_name = var_name;
      _problem->addAuxKernel("MaterialStdVectorAux", aux_kernel_name, params);
    }
}
