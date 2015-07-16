/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PFFracBulkRateMaterial.h"

template<>
InputParameters validParams<PFFracBulkRateMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Material properties used in phase-field fracture damage evolution kernel");
  params.addParam<FunctionName>("function", "", "Function describing energy release rate type parameter distribution");
  params.addParam<Real>("gc", 1.0, "Energy release rate type parameter");

  return params;
}

PFFracBulkRateMaterial::PFFracBulkRateMaterial(const InputParameters & parameters) :
  Material(parameters),
  _gc(getParam<Real>("gc")),
  _gc_prop(declareProperty<Real>("gc_prop"))
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


// DEPRECATED CONSTRUCTOR
PFFracBulkRateMaterial::PFFracBulkRateMaterial(const std::string & deprecated_name, InputParameters parameters) :
  Material(deprecated_name, parameters),
  _gc(getParam<Real>("gc")),
  _gc_prop(declareProperty<Real>("gc_prop"))
{
}
