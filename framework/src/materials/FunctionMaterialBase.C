//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionMaterialBase.h"

template <bool is_ad>
InputParameters
FunctionMaterialBase<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Material providing a functionalized/parsed material property");

  params.addDeprecatedParam<std::string>(
      "f_name", "Name of the parsed material property", "f_name is deprecated, use property_name");
  // TODO Make required once deprecation is handled, see #19119
  params.addParam<std::string>("property_name", "F", "Name of the parsed material property");
  return params;
}

template <bool is_ad>
FunctionMaterialBase<is_ad>::FunctionMaterialBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _F_name(getRenamedParam<std::string>("f_name", "property_name")),
    _prop_F(&declareGenericProperty<Real, is_ad>(_F_name))
{
  // fetch names and numbers of all coupled variables
  _mapping_is_unique = true;
  for (std::set<std::string>::const_iterator it = _pars.coupledVarsBegin();
       it != _pars.coupledVarsEnd();
       ++it)
  {
    // find the variable in the list of coupled variables
    auto vars = _coupled_vars.find(*it);

    // no MOOSE variable was provided for this coupling, add to a list of variables set to constant
    // default values
    if (vars == _coupled_vars.end())
    {
      if (_pars.hasDefaultCoupledValue(*it))
        _arg_constant_defaults.push_back(*it);
      continue;
    }

    // check if we have a 1:1 mapping between parameters and variables
    if (vars->second.size() != 1)
      _mapping_is_unique = false;

    // iterate over all components
    for (unsigned int j = 0; j < vars->second.size(); ++j)
    {
      // make sure each nonlinear variable is coupled in only once
      if (std::find(_arg_names.begin(), _arg_names.end(), vars->second[j]->name()) !=
          _arg_names.end())
        mooseError("A nonlinear variable can only be coupled in once.");

      // insert the map values
      // unsigned int number = vars->second[j]->number();
      unsigned int number = coupled(*it, j);
      _arg_names.push_back(vars->second[j]->name());
      _arg_numbers.push_back(number);
      _arg_param_names.push_back(*it);
      if (_mapping_is_unique)
        _arg_param_numbers.push_back(-1);
      else
        _arg_param_numbers.push_back(j);

      // populate number -> arg index lookup table
      unsigned int idx = libMeshVarNumberRemap(number);
      if (idx >= _arg_index.size())
        _arg_index.resize(idx + 1, -1);

      _arg_index[idx] = _args.size();

      // get variable value
      _args.push_back(&coupledGenericValue(*it, j));
    }
  }

  _nargs = _arg_names.size();
}

template <bool is_ad>
const GenericVariableValue<is_ad> &
FunctionMaterialBase<is_ad>::coupledGenericValue(const std::string & var_name, unsigned int comp)
{
  return coupledValue(var_name, comp);
}

template <>
const GenericVariableValue<true> &
FunctionMaterialBase<true>::coupledGenericValue(const std::string & var_name, unsigned int comp)
{
  return adCoupledValue(var_name, comp);
}

// explicit instantiation
template class FunctionMaterialBase<false>;
template class FunctionMaterialBase<true>;
