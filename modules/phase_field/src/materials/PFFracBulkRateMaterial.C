/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PFFracBulkRateMaterial.h"

template <>
InputParameters
validParams<PFFracBulkRateMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Material properties used in phase-field fracture damage evolution kernel");
  params.addParam<FunctionName>(
      "function", "Function describing energy release rate type parameter distribution");
  params.addParam<Real>("gc", 1.0, "Energy release rate type parameter");

  return params;
}

PFFracBulkRateMaterial::PFFracBulkRateMaterial(const InputParameters & parameters)
  : Material(parameters),
    _gc(getParam<Real>("gc")),
    _gc_prop(declareProperty<Real>("gc_prop")),
    _function_prop(isParamValid("function") ? &getFunction("function") : NULL)
{
}

void
PFFracBulkRateMaterial::initQpStatefulProperties()
{
  _gc_prop[_qp] = _gc;
}

void
PFFracBulkRateMaterial::computeQpProperties()
{
  _gc_prop[_qp] = _gc;
  /**
   * This function computes heterogeneous gc
   * User should override this function if heterogenities needs consideration
   */
  getProp();
}

void
PFFracBulkRateMaterial::getProp()
{
  if (_function_prop != NULL)
    _gc_prop[_qp] = _function_prop->value(_t, _q_point[_qp]);
}
