//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeParsedMaterialHelper.h"
#include "Conversion.h"

#include <deque>

#include "libmesh/quadrature.h"

template <bool is_ad>
InputParameters
DerivativeParsedMaterialHelperTempl<is_ad>::validParams()
{
  InputParameters params = ParsedMaterialHelper<is_ad>::validParams();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  params.addParam<unsigned int>("derivative_order", 3, "Maximum order of derivatives taken");
  params.addParam<std::vector<SymbolName>>(
      "additional_derivative_symbols",
      "A list of additional (non-variable) symbols (such as material property or postprocessor "
      "names) to take derivatives w.r.t.");
  return params;
}

template <bool is_ad>
DerivativeParsedMaterialHelperTempl<is_ad>::DerivativeParsedMaterialHelperTempl(
    const InputParameters & parameters, VariableNameMappingMode map_mode)
  : ParsedMaterialHelper<is_ad>(parameters, map_mode),
    _derivative_order(this->template getParam<unsigned int>("derivative_order")),
    _dmatvar_base("matpropautoderiv"),
    _dmatvar_index(0)
{
}

template <bool is_ad>
void
DerivativeParsedMaterialHelperTempl<is_ad>::functionsPostParse()
{
  // set up the list of all FParser symbols to derive w.r.t.
  auto additional_derivative_symbols =
      this->template getParam<std::vector<SymbolName>>("additional_derivative_symbols");

  // build a list of indices into _symbol_table for all symbols we want to derive w.r.t.
  // we start with all MOOSE variables, which we _always_ take the derivatives w.r.t.
  _derivative_symbol_table.reserve(_nargs + additional_derivative_symbols.size());
  for (unsigned i = 0; i < _nargs; ++i)
    _derivative_symbol_table.push_back(i);

  // then we add the additional derivative symbols
  for (auto & ads : additional_derivative_symbols)
  {
    // find the index of the symbol if it exists
    auto it = std::find(_symbol_names.begin(), _symbol_names.end(), ads);
    if (it == _symbol_names.end())
      this->paramError("additional_derivative_symbols", "Invalid symbol name '", ads, "'.");
    auto idx = std::distance(_symbol_names.begin(), it);

    // make sure it is not a MOOSE variable (we already added those) - their index is < nargs
    if (idx < _nargs)
      this->paramError(
          "additional_derivative_symbols",
          "Symbol name '",
          ads,
          "' is a coupled variable. Derivatives w.r.t. coupled variables are always taken "
          "and they must not be specified again.");

    // check that the user did not specify any duplicates in the additional_derivative_symbols
    // parameter
    if (std::count(
            additional_derivative_symbols.begin(), additional_derivative_symbols.end(), ads) > 1)
      this->paramError(
          "additional_derivative_symbols", "Symbol name '", ads, "' was given more than once.");

    // if all checks passed, add the symbol index to the list
    _derivative_symbol_table.push_back(idx);
  }

  // optimize base function
  ParsedMaterialHelper<is_ad>::functionsOptimize();

  // generate derivatives
  assembleDerivatives();

  // force a value update to get the property at least once and register it for the dependencies
  for (auto & mpd : _mat_prop_descriptors)
    mpd.value();

  // batch compilation
}

template <bool is_ad>
void
DerivativeParsedMaterialHelperTempl<is_ad>::recurseMatProps(
    unsigned int var, unsigned int order, const MatPropDescriptorList & parent_mpd_list)
{
  // quit if we have exceeded the requested derivative order
  if (order > _derivative_order)
    return;

  // FParser symbol we are deriving w.r.t.
  auto derivative_var = _derivative_symbol_table[var];
  auto derivative_symbol = _symbol_names[derivative_var];

  // generate parent material property descriptors derivatives
  MatPropDescriptorList mpd_list;
  for (const auto & parent_mpd : parent_mpd_list)
  {
    // if this material property does not depend on the variable we are deriving w.r.t. skip it
    if (!parent_mpd.dependsOn(derivative_symbol))
      continue;

    // otherwise add it to _mat_prop_descriptors
    FunctionMaterialPropertyDescriptor<is_ad> mpd(parent_mpd);
    mpd.addDerivative(derivative_symbol);

    // create a new symbol name for it
    std::string newvarname = _dmatvar_base + Moose::stringify(_dmatvar_index++);
    mpd.setSymbolName(newvarname);

    // add the new mpd and register it as the current variable derivative of the parent mpd
    _bulk_rules.emplace_back(parent_mpd.getSymbolName(), derivative_symbol, newvarname);

    // append to list
    mpd_list.push_back(mpd);
  }

  // append material property descriptors
  for (const auto & mpd : mpd_list)
    _mat_prop_descriptors.push_back(mpd);

  // go one order deeper
  for (unsigned int i = var; i < _derivative_symbol_table.size(); ++i)
    recurseMatProps(i, order + 1, mpd_list);
}

template <bool is_ad>
void
DerivativeParsedMaterialHelperTempl<is_ad>::recurseDerivative(unsigned int var,
                                                              unsigned int order,
                                                              const Derivative & parent_derivative)
{
  // quit if we have exceeded the requested derivative order
  if (order > _derivative_order)
    return;

  // FParser symbol we are deriving w.r.t.
  auto derivative_var = _derivative_symbol_table[var];
  auto derivative_symbol = _symbol_names[derivative_var];

  // current derivative starts off of the parent function
  Derivative current;
  current._darg_names = parent_derivative._darg_names;

  // the moose variable name goes into the derivative property name
  current._darg_names.push_back(derivative_var < _nargs ? _arg_names[derivative_var]
                                                        : derivative_symbol);

  current._F = std::make_shared<SymFunction>(*parent_derivative._F);

  // execute derivative
  if (current._F->AutoDiff(derivative_symbol) != -1)
    mooseError("Failed to take order ", order, " derivative in material ", _name);

  // optimize
  if (!_disable_fpoptimizer)
    current._F->Optimize();

  // proceed only if the derivative is not zero
  if (!current._F->isZero() || is_ad)
  {
    // compile
    if (_enable_jit && !current._F->JITCompile())
      mooseInfo("Failed to JIT compile expression, falling back to byte code interpretation.");

    // go one order deeper
    for (unsigned int i = var; i < _derivative_symbol_table.size(); ++i)
      recurseDerivative(i, order + 1, current);

    // set up a material property for the derivative
    current._mat_prop =
        &this->template declarePropertyDerivative<Real, is_ad>(_F_name, current._darg_names);

    // save off current derivative
    _derivatives.push_back(current);
  }
}

/**
 * Perform construction of all requested derivatives.
 */
template <bool is_ad>
void
DerivativeParsedMaterialHelperTempl<is_ad>::assembleDerivatives()
{
  // need to check for zero derivatives here, otherwise at least one order is generated
  if (_derivative_order < 1)
    return;

  // if we are not on thread 0 we fetch all data from the thread 0 copy that already did all the
  // work
  if (_tid > 0)
  {
    // get the master object from thread 0
    const MaterialWarehouse & material_warehouse = _fe_problem.getMaterialWarehouse();
    const MooseObjectWarehouse<MaterialBase> & warehouse = material_warehouse[_material_data_type];

    MooseSharedPointer<DerivativeParsedMaterialHelperTempl> master =
        MooseSharedNamespace::dynamic_pointer_cast<DerivativeParsedMaterialHelperTempl>(
            warehouse.getActiveObject(name()));

    // copy parsers and declare properties
    for (const auto & D : master->_derivatives)
    {
      Derivative newderivative;
      newderivative._mat_prop =
          &this->template declarePropertyDerivative<Real, is_ad>(_F_name, D._darg_names);
      newderivative._F = std::make_shared<SymFunction>(*D._F);
      _derivatives.push_back(newderivative);
    }

    // copy coupled material properties
    auto start = _mat_prop_descriptors.size();
    for (MooseIndex(master->_mat_prop_descriptors) i = start;
         i < master->_mat_prop_descriptors.size();
         ++i)
    {
      FunctionMaterialPropertyDescriptor<is_ad> newdescriptor(master->_mat_prop_descriptors[i],
                                                              this);
      _mat_prop_descriptors.push_back(newdescriptor);
    }

    // size parameter buffer
    _func_params.resize(master->_func_params.size());
    return;
  }

  // generate all coupled material property mappings
  for (unsigned int i = 0; i < _derivative_symbol_table.size(); ++i)
    recurseMatProps(i, 1, _mat_prop_descriptors);

  // bulk register material property derivative rules to avoid repeated calls
  // to the (slow) AddVariable method
  if (!_bulk_rules.empty())
  {
    std::string vars = _bulk_rules[0]._child;
    for (MooseIndex(_bulk_rules) i = 1; i < _bulk_rules.size(); ++i)
      vars += ',' + _bulk_rules[i]._child;
    _func_F->AddVariable(vars);

    for (const auto & rule : _bulk_rules)
      _func_F->RegisterDerivative(rule._parent, rule._var, rule._child);
  }

  // generate all derivatives
  Derivative root;
  root._F = _func_F;
  root._mat_prop = nullptr;
  for (unsigned int i = 0; i < _derivative_symbol_table.size(); ++i)
    recurseDerivative(i, 1, root);

  // increase the parameter buffer to provide storage for the material property derivatives
  _func_params.resize(_nargs + _mat_prop_descriptors.size() + _postprocessor_values.size());
}

template <bool is_ad>
void
DerivativeParsedMaterialHelperTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
DerivativeParsedMaterialHelperTempl<is_ad>::computeQpProperties()
{
  ParsedMaterialHelper<is_ad>::computeQpProperties();

  // set derivatives
  for (auto & D : _derivatives)
    (*D._mat_prop)[_qp] = evaluate(D._F, _name);
}

// explicit instantiation
template class DerivativeParsedMaterialHelperTempl<false>;
template class DerivativeParsedMaterialHelperTempl<true>;
