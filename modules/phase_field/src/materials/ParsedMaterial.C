/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ParsedMaterial.h"

template<>
InputParameters validParams<ParsedMaterial>()
{
  InputParameters params = ParsedMaterialHelper<FunctionMaterialBase>::validParams();
  params += validParams<ParsedMaterialBase>();
  params.addClassDescription("Parsed Function Material.");
  return params;
}

ParsedMaterial::ParsedMaterial(const std::string & name,
                               InputParameters parameters) :
    ParsedMaterialHelper<FunctionMaterialBase>(name, parameters, USE_MOOSE_NAMES),
    ParsedMaterialBase(name, parameters)
{
  // Build function and optimize
  functionParse(_function,
                _constant_names, _constant_expressions,
                _mat_prop_names,
                _tol_names, _tol_values);
}
