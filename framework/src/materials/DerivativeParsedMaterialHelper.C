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
validParams<DerivativeParsedMaterialHelper>()
{
  InputParameters params = validParams<ParsedMaterialHelper>();
  params.addClassDescription("Parsed Function Material with automatic derivatives.");
  params.addDeprecatedParam<bool>("third_derivatives",
                                  "Flag to indicate if third derivatives are needed",
                                  "Use derivative_order instead.");
  params.addParam<unsigned int>("derivative_order", 3, "Maximum order of derivatives taken");

  return params;
}

DerivativeParsedMaterialHelper::DerivativeParsedMaterialHelper(const InputParameters & parameters,
                                                               VariableNameMappingMode map_mode)
  : ParsedMaterialHelper(parameters, map_mode),
    //_derivative_order(getParam<unsigned int>("derivative_order"))
    _derivative_order(isParamValid("third_derivatives")
                          ? (getParam<bool>("third_derivatives") ? 3 : 2)
                          : getParam<unsigned int>("derivative_order")),
    _dmatvar_base("matpropautoderiv"),
    _dmatvar_index(0)
{
}

void
DerivativeParsedMaterialHelper::functionsPostParse()
{
  // optimize base function
  ParsedMaterialHelper::functionsOptimize();

  // generate derivatives
  assembleDerivatives();

  // force a value update to get the property at least once and register it for the dependencies
  for (auto & mpd : _mat_prop_descriptors)
    mpd.value();
}

void
DerivativeParsedMaterialHelper::recurseMatProps(unsigned int var,
                                                unsigned int order,
                                                const MatPropDescriptorList & parent_mpd_list)
{
  // quit if we have exceeded the requested derivative order
  if (order > _derivative_order)
    return;

  // generate parent material property descriptors derivatives
  MatPropDescriptorList mpd_list;
  for (const auto & parent_mpd : parent_mpd_list)
  {
    // if this material property does not depend on the variable we are deriving w.r.t. skip it
    if (!parent_mpd.dependsOn(_arg_names[var]))
      continue;

    // otherwise add it to _mat_prop_descriptors
    FunctionMaterialPropertyDescriptor mpd(parent_mpd);
    mpd.addDerivative(_arg_names[var]);

    // create a new symbol name for it
    std::string newvarname = _dmatvar_base + Moose::stringify(_dmatvar_index++);
    mpd.setSymbolName(newvarname);

    // add the new mpd and register it as the current variable derivative of the parent mpd
    _func_F->AddVariable(newvarname);
    _func_F->RegisterDerivative(parent_mpd.getSymbolName(), _variable_names[var], newvarname);

    // append to list
    mpd_list.push_back(mpd);
  }

  // append material property descriptors
  for (const auto & mpd : mpd_list)
    _mat_prop_descriptors.push_back(mpd);

  // go one order deeper
  for (unsigned int i = var; i < _nargs; ++i)
    recurseMatProps(i, order + 1, mpd_list);
}

void
DerivativeParsedMaterialHelper::recurseDerivative(unsigned int var,
                                                  unsigned int order,
                                                  const Derivative & parent_derivative)
{
  // quit if we have exceeded the requested derivative order
  if (order > _derivative_order)
    return;

  // variable we are deriving w.r.t.
  auto derivative_var = _variable_names[var];

  // current derivative starts off of the parent function
  Derivative current;
  current._darg_names = parent_derivative._darg_names;
  // the moose variable name goes into the derivative property name
  current._darg_names.push_back(_arg_names[var]);
  current._F = ADFunctionPtr(new ADFunction(*parent_derivative._F));

  // execute derivative
  if (current._F->AutoDiff(derivative_var) != -1)
    mooseError("Failed to take order ", order, " derivative in material ", _name);

  // optimize
  if (!_disable_fpoptimizer)
    current._F->Optimize();

  // proceed only if the derivative is not zero
  if (!current._F->isZero())
  {
    // compile
    if (_enable_jit && !current._F->JITCompile())
      mooseInfo("Failed to JIT compile expression, falling back to byte code interpretation.");

    // go one order deeper
    for (unsigned int i = var; i < _nargs; ++i)
      recurseDerivative(i, order + 1, current);

    // set up a material property for the derivative
    current._mat_prop = &declarePropertyDerivative<Real>(_F_name, current._darg_names);

    // save off current derivative
    _derivatives.push_back(current);
  }
}

/**
 * Perform construction of all requested derivatives.
 */
void
DerivativeParsedMaterialHelper::assembleDerivatives()
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
    const MooseObjectWarehouse<Material> & warehouse = material_warehouse[_material_data_type];

    MooseSharedPointer<DerivativeParsedMaterialHelper> master =
        MooseSharedNamespace::dynamic_pointer_cast<DerivativeParsedMaterialHelper>(
            warehouse.getActiveObject(name()));

    // copy parsers and declare properties
    for (const auto & D : master->_derivatives)
    {
      Derivative newderivative;
      newderivative._mat_prop = &declarePropertyDerivative<Real>(_F_name, D._darg_names);
      newderivative._F = ADFunctionPtr(new ADFunction(*D._F));
      _derivatives.push_back(newderivative);
    }

    // copy coupled material properties
    auto start = _mat_prop_descriptors.size();
    for (MooseIndex(master->_mat_prop_descriptors) i = start;
         i < master->_mat_prop_descriptors.size();
         ++i)
    {
      FunctionMaterialPropertyDescriptor newdescriptor(master->_mat_prop_descriptors[i], this);
      _mat_prop_descriptors.push_back(newdescriptor);
    }

    // size parameter buffer
    _func_params.resize(master->_func_params.size());
    return;
  }

  // generate all coupled material property mappings
  for (unsigned int i = 0; i < _nargs; ++i)
    recurseMatProps(i, 1, _mat_prop_descriptors);

  // generate all derivatives
  Derivative root;
  root._F = _func_F;
  root._mat_prop = nullptr;
  for (unsigned int i = 0; i < _nargs; ++i)
    recurseDerivative(i, 1, root);

  // increase the parameter buffer to provide storage for the material property derivatives
  _func_params.resize(_nargs + _mat_prop_descriptors.size());
}

void
DerivativeParsedMaterialHelper::initQpStatefulProperties()
{
  if (_prop_F)
    (*_prop_F)[_qp] = 0.0;

  for (auto & D : _derivatives)
    (*D._mat_prop)[_qp] = 0.0;
}

void
DerivativeParsedMaterialHelper::computeQpProperties()
{
  // fill the parameter vector, apply tolerances
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    if (_tol[i] < 0.0)
      _func_params[i] = (*_args[i])[_qp];
    else
    {
      Real a = (*_args[i])[_qp];
      _func_params[i] = a < _tol[i] ? _tol[i] : (a > 1.0 - _tol[i] ? 1.0 - _tol[i] : a);
    }
  }

  // insert material property values
  auto nmat_props = _mat_prop_descriptors.size();
  for (MooseIndex(_mat_prop_descriptors) i = 0; i < nmat_props; ++i)
    _func_params[i + _nargs] = _mat_prop_descriptors[i].value()[_qp];

  // set function value
  if (_prop_F)
    (*_prop_F)[_qp] = evaluate(_func_F);

  // set derivatives
  for (auto & D : _derivatives)
    (*D._mat_prop)[_qp] = evaluate(D._F);
}
