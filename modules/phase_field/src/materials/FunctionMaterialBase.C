/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FunctionMaterialBase.h"

template <>
InputParameters
validParams<FunctionMaterialBase>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Material to provide a function (such as a free energy)");
  params.addParam<std::string>(
      "f_name",
      "F",
      "Base name of the free energy function (used to name the material properties)");
  return params;
}

FunctionMaterialBase::FunctionMaterialBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _F_name(getParam<std::string>("f_name")),
    _prop_F(&declareProperty<Real>(_F_name))
{
  // fetch names and numbers of all coupled variables
  _mapping_is_unique = true;
  for (std::set<std::string>::const_iterator it = _pars.coupledVarsBegin();
       it != _pars.coupledVarsEnd();
       ++it)
  {
    // find the variable in the list of coupled variables
    std::map<std::string, std::vector<MooseVariable *>>::iterator vars = _coupled_vars.find(*it);

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

      // populate number -> arg index lookup table
      unsigned int idx = libMeshVarNumberRemap(number);
      if (idx >= _arg_index.size())
        _arg_index.resize(idx + 1, -1);

      _arg_index[idx] = _args.size();

      // get variable value
      _args.push_back(&coupledValue(*it, j));
    }
  }

  _nargs = _arg_names.size();
}
