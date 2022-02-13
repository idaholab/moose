//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledValueFunctionMaterial.h"
#include "Function.h"

registerMooseObject("MooseApp", CoupledValueFunctionMaterial);
registerMooseObject("MooseApp", ADCoupledValueFunctionMaterial);

template <bool is_ad>
InputParameters
CoupledValueFunctionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a function value from coupled variables");
  params.addRequiredParam<FunctionName>("function",
                                        "Coupled function to evaluate with values from v");
  MultiMooseEnum parameter_order_enum("X Y Z T");
  params.addParam<MultiMooseEnum>("parameter_order",
                                  parameter_order_enum,
                                  "If provided, an entry per couple variable specifies "
                                  "the function argument it should apply to.");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "Output property name");
  params.addCoupledVar(
      "v",
      "List of up to four coupled variables that are substituted for x,y,z, and t "
      "in the coupled function (or the order chosen using the `parameter_order` parameter)");
  return params;
}

template <bool is_ad>
CoupledValueFunctionMaterialTempl<is_ad>::CoupledValueFunctionMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop(declareGenericProperty<Real, is_ad>(getParam<MaterialPropertyName>("prop_name"))),
    _function(getFunction("function")),
    _vals(coupledGenericValues<is_ad>("v")),
    _nvals(coupledComponents("v"))
{
  if (_nvals > 4)
    paramError("v", "Couple a maximum of four variables");

  const auto & param_order = getParam<MultiMooseEnum>("parameter_order");

  // no custom order is specified, use x,y,z,t
  if (param_order.size() == 0)
    for (const auto i : make_range(_nvals))
      _order.push_back(i);
  else if (param_order.size() == _nvals)
  {
    for (const auto i : make_range(_nvals))
      _order.push_back(param_order.get(i));

    std::set<unsigned int> check_doubles(_order.begin(), _order.end());
    if (check_doubles.size() != _nvals)
      paramError("parameter_order", "You must not repeat any positions.");
  }
  else
    paramError("parameter_order",
               "Specify either as many items as coupled variables, or none at all for the default "
               "order of x,y,z,t.");
}

template <bool is_ad>
void
CoupledValueFunctionMaterialTempl<is_ad>::computeQpProperties()
{
  MooseADWrapper<Point, is_ad> p;
  GenericReal<is_ad> t = 0.0;

  for (const auto i : make_range(_nvals))
  {
    const auto & j = _order[i];
    if (j < 3)
      p(j) = (*_vals[i])[_qp];
    else
      t = (*_vals[i])[_qp];
  }

  _prop[_qp] = _function.value(t, p);
}

template class CoupledValueFunctionMaterialTempl<false>;
template class CoupledValueFunctionMaterialTempl<true>;
