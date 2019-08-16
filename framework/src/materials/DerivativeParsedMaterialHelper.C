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

template <>
InputParameters
validParams<DerivativeParsedMaterialHelper<> >()
{
  InputParameters params = validParams<ParsedMaterialHelper<> >();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  params.addDeprecatedParam<bool>("third_derivatives",
                                  "Flag to indicate if third derivatives are needed",
                                  "Use derivative_order instead.");
  params.addParam<unsigned int>("derivative_order", 3, "Maximum order of derivatives taken");

  return params;
}

template <typename T>
DerivativeParsedMaterialHelper<T>::DerivativeParsedMaterialHelper(const InputParameters & parameters,
                                                               typename ParsedMaterialHelper<T>::VariableNameMappingMode map_mode)
  : ParsedMaterialHelper<T>(parameters, map_mode),
    //_derivative_order(getParam<unsigned int>("derivative_order"))
    _derivative_order(this->isParamValid("third_derivatives")
                          ? (getParam<bool>("third_derivatives") ? 3 : 2)
                          : getParam<unsigned int>("derivative_order")),
    _dmatvar_base("matpropautoderiv"),
    _dmatvar_index(0)
{
}

template <typename T>
void
DerivativeParsedMaterialHelper<T>::functionsPostParse()
{
  // optimize base function
  ParsedMaterialHelper<T>::functionsOptimize();

  // generate derivatives
  assembleDerivatives();

  // force a value update to get the property at least once and register it for the dependencies
  for (auto & mpd : ParsedMaterialHelper<T>::_real_mat_prop_descriptors)
    mpd.value();
  for (auto & mpd : ParsedMaterialHelper<T>::_real_vec_mat_prop_descriptors)
    mpd.value();
  for (auto & mpd : ParsedMaterialHelper<T>::_r_two_tens_mat_prop_descriptors)
    mpd.value();
}

template <typename T>
void
DerivativeParsedMaterialHelper<T>::recurseMatProps(unsigned int var,
                                                unsigned int order,
                                                const typename ParsedMaterialHelper<T>::MatPropDescriptorList & parent_mpd_list)
{
  // quit if we have exceeded the requested derivative order
  if (order > _derivative_order)
    return;

  // generate parent material property descriptors derivatives
  typename ParsedMaterialHelper<T>::MatPropDescriptorList mpd_list;
  for (const auto & parent_mpd : parent_mpd_list)
  {
    // if this material property does not depend on the variable we are deriving w.r.t. skip it
    if (!parent_mpd.dependsOn(this->_arg_names[var]))
    {
      continue;
    }
    // otherwise add it to _mat_prop_descriptors
    FunctionMaterialPropertyDescriptor<> mpd(parent_mpd);
    mpd.addDerivative(this->_arg_names[var]);

    // create a new symbol name for it
    std::string newvarname = _dmatvar_base + Moose::stringify(_dmatvar_index++);
    mpd.setSymbolName(newvarname);

    // add the new mpd and register it as the current variable derivative of the parent mpd
    _bulk_rules.emplace_back(parent_mpd.getSymbolName(), this->_variable_names[var], newvarname);

    // append to list
    mpd_list.push_back(mpd);
  }

  // append material property descriptors
  for (const auto & mpd : mpd_list)
    this->_real_mat_prop_descriptors.push_back(mpd);

  // go one order deeper
  for (unsigned int i = var; i < this->_nargs; ++i)
    recurseMatProps(i, order + 1, mpd_list);
}

template <typename T>
void
DerivativeParsedMaterialHelper<T>::recurseDerivative(unsigned int var,
                                                  unsigned int order,
                                                  const Derivative & parent_derivative)
{
  // quit if we have exceeded the requested derivative order
  if (order > _derivative_order)
    return;

  // variable we are deriving w.r.t.
  auto derivative_var = this->_variable_names[var];

  // current derivative starts off of the parent function
  Derivative current;
  bool is_zero = true;
  current._F.resize(this->_func_F.size());
  current._darg_names = parent_derivative._darg_names;
  // the moose variable name goes into the derivative property name
  current._darg_names.push_back(this->_arg_names[var]);

  for(unsigned int i = 0; i < this->_func_F.size(); ++i)
  {
    current._F[i] = FunctionParserUtils::ADFunctionPtr(new FunctionParserUtils::ADFunction(*parent_derivative._F[i]));

    // execute derivative
    if (current._F[i]->AutoDiff(derivative_var) != -1)
      mooseError("Failed to take order ", order, " derivative in material ", this->_name);

    // optimize
    if (!this->_disable_fpoptimizer)
      current._F[i]->Optimize();
    if(!current._F[i]->isZero())
      is_zero = false;
  }

  // don't proceed only if the derivative of all components is zero
  if (!is_zero)
  {
    for(unsigned int i = 0; i < current._F.size(); ++i)
    {
      // compile
      if (this->_enable_jit && !current._F[i]->JITCompile())
        mooseInfo("Failed to JIT compile expression, falling back to byte code interpretation.");
    }


    // go one order deeper
    for (unsigned int i = var; i < this->_nargs; ++i)
      recurseDerivative(i, order + 1, current);

    // set up a material property for the derivative
    current._mat_prop = &FunctionMaterialBase<T>::template declarePropertyDerivative<T>(this->_F_name, current._darg_names);

    // save off current derivative
    _derivatives.push_back(current);
  }
}

/**
 * Perform construction of all requested derivatives.
 */
 template <typename T>
void
DerivativeParsedMaterialHelper<T>::assembleDerivatives()
{
  // need to check for zero derivatives here, otherwise at least one order is generated
  if (_derivative_order < 1)
    return;

  // if we are not on thread 0 we fetch all data from the thread 0 copy that already did all the
  // work
  if (this->_tid > 0)
  {
    // get the master object from thread 0
    const MaterialWarehouse & material_warehouse = this->_fe_problem.getMaterialWarehouse();
    const MooseObjectWarehouse<Material> & warehouse = material_warehouse[this->_material_data_type];

    MooseSharedPointer<DerivativeParsedMaterialHelper> master =
        MooseSharedNamespace::dynamic_pointer_cast<DerivativeParsedMaterialHelper<T> >(
            warehouse.getActiveObject(this->name()));

    // copy parsers and declare properties
    for (const auto & D : master->_derivatives)
    {
      Derivative newderivative;
      newderivative._F.resize(this->_func_F.size());
      newderivative._mat_prop = &FunctionMaterialBase<T>::template declarePropertyDerivative<T>(this->_F_name, D._darg_names);
      for(unsigned int i = 0; i < this->_func_F.size(); ++i)
      {
        newderivative._F[i] = FunctionParserUtils::ADFunctionPtr(new FunctionParserUtils::ADFunction(*D._F[i]));
      }
      _derivatives.push_back(newderivative);
    }

    // copy coupled material properties
    auto start = this->_real_mat_prop_descriptors.size();
    for (MooseIndex(master->_real_mat_prop_descriptors) i = start;
         i < master->_real_mat_prop_descriptors.size();
         ++i)
    {
      FunctionMaterialPropertyDescriptor<> newdescriptor(master->_real_mat_prop_descriptors[i], this);
      this->_real_mat_prop_descriptors.push_back(newdescriptor);
    }

    // size parameter buffer
    this->_func_params.resize(master->_func_params.size());
    return;
  }

  // generate all coupled material property mappings
  for (unsigned int i = 0; i < this->_nargs; ++i)
    recurseMatProps(i, 1, this->_real_mat_prop_descriptors);

  for(unsigned int j = 0; j < this->_func_F.size(); ++j)
  {
    // bulk register material property derivative rules to avoid repeated calls
    // to the (slow) AddVariable method
    if (!_bulk_rules.empty())
    {
      std::string vars = _bulk_rules[0]._child;
      for (MooseIndex(_bulk_rules) i = 1; i < _bulk_rules.size(); ++i)
        vars += ',' + _bulk_rules[i]._child;
      this->_func_F[j]->AddVariable(vars);

      for (const auto & rule : _bulk_rules)
        this->_func_F[j]->RegisterDerivative(rule._parent, rule._var, rule._child);
    }
  }
    // generate all derivatives
  Derivative root;
  root._F.resize(this->_func_F.size());
  for(unsigned int i = 0; i < this->_func_F.size(); ++i)
    root._F[i] = this->_func_F[i];
  root._mat_prop = nullptr;
  for (unsigned int i = 0; i < this->_nargs; ++i)
    recurseDerivative(i, 1, root);

  // increase the parameter buffer to provide storage for the material property derivatives
  this->_func_params.resize(this->_nargs + this->_r_two_tens_mat_prop_descriptors.size() + this->_real_vec_mat_prop_descriptors.size() + this->_real_mat_prop_descriptors.size());
}

template <>
void
DerivativeParsedMaterialHelper<Real>::initQpStatefulProperties()
{
  if (_prop_F)
    (*_prop_F)[_qp] = 0.0;

  for (auto & D : _derivatives)
    (*D._mat_prop)[_qp] = 0.0;
}

template<>
void
DerivativeParsedMaterialHelper<RealVectorValue>::initQpStatefulProperties()
{
  if (_prop_F)
    for(unsigned int i = 0; i < this->_func_F.size(); ++i)
      ((*_prop_F)[_qp])(i) = 0.0;

  for (auto & D : _derivatives)
    for(unsigned int i = 0; i < this->_func_F.size(); ++i)
      ((*D._mat_prop)[_qp])(i) = 0.0;
}

template <>
void
DerivativeParsedMaterialHelper<Real>::functionValue()
{
  if (_prop_F)
    (*_prop_F)[_qp] = evaluate(this->_func_F[0]);

  // set derivatives
  for (auto & D : _derivatives)
    (*D._mat_prop)[_qp] = evaluate(D._F[0]);
}

template <>
void
DerivativeParsedMaterialHelper<RealVectorValue>::functionValue()
{
  if (_prop_F)
    for(unsigned int i = 0; i < this->_func_F.size(); ++i)
      ((*_prop_F)[_qp])(i) = evaluate(this->_func_F[i]);

  // set derivatives
  for (auto & D : _derivatives)
    for(unsigned int i = 0; i < this->_func_F.size(); ++i)
      ((*D._mat_prop)[_qp])(i) = evaluate(D._F[i]);
}

template class DerivativeParsedMaterialHelper<Real>;
template class DerivativeParsedMaterialHelper<RealVectorValue>;
