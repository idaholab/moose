//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatSource.h"

registerKokkosResidualObject("HeatTransferApp", KokkosHeatSource);

InputParameters
KokkosHeatSource::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addClassDescription("Volumetric heat source term $-(\\psi_i, f)$ of the thermal energy "
                             "conservation equation.");
  params.addParam<Real>("value", 1.0, "Value of heat source. Multiplied by function if present.");
  params.addParam<FunctionName>("function", "1", "Function describing the volumetric heat source");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the heat source");
  params.declareControllable("value");
  return params;
}

KokkosHeatSource::KokkosHeatSource(const InputParameters & parameters)
  : KernelValue(parameters),
    _scale(getParam<Real>("value")),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _function(getKokkosFunction("function"))
{
}
