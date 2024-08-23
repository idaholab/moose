//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialVectorGradAuxKernelAction.h"
#include "Factory.h"
#include "Parser.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "MooseMesh.h"

registerMooseAction("PhaseFieldApp", MaterialVectorGradAuxKernelAction, "add_aux_kernel");

InputParameters
MaterialVectorGradAuxKernelAction::validParams()
{
  InputParameters params = MaterialVectorAuxKernelAction::validParams();
  params.addClassDescription("Outputs all components of the gradient of the real standard "
                             "vector-valued properties specified");
  return params;
}

MaterialVectorGradAuxKernelAction::MaterialVectorGradAuxKernelAction(const InputParameters & params)
  : MaterialVectorAuxKernelAction(params)
{
}

void
MaterialVectorGradAuxKernelAction::act()
{
  if (_num_prop != _num_var)
    paramError("property", "variable_base and property must be vectors of the same size");

  // mesh dimension required for gradient variables
  unsigned int dim = _mesh->dimension();
  // For Specifying the components of the gradient terms
  const std::vector<char> suffix = {'x', 'y', 'z'};

  for (unsigned int gr = 0; gr < _grain_num; ++gr)
    for (unsigned int val = 0; val < _num_var; ++val)
      for (unsigned int x = 0; x < dim; ++x)
      {
        std::string var_name = _var_name_base[val] + Moose::stringify(gr) + "_" + suffix[x];

        InputParameters params = _factory.getValidParams("MaterialStdVectorRealGradientAux");
        params.set<AuxVariableName>("variable") = var_name;
        params.set<MaterialPropertyName>("property") = _prop[val];
        params.set<unsigned int>("component") = x;
        params.set<unsigned int>("index") = gr;
        params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

        std::string aux_kernel_name = var_name;
        _problem->addAuxKernel("MaterialStdVectorRealGradientAux", aux_kernel_name, params);
      }
}
