//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementQualityAux.h"

#include "ElementQualityChecker.h"

registerMooseObject("MooseApp", ElementQualityAux);

InputParameters
ElementQualityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Generates a field containing the quality metric for each element.  "
                             "Useful for visualizing mesh quality.");

  params.addRequiredParam<MooseEnum>(
      "metric", ElementQualityChecker::QualityMetricType(), "The quality metric to use.");

  return params;
}

ElementQualityAux::ElementQualityAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _metric_type(getParam<MooseEnum>("metric").getEnum<libMesh::ElemQuality>())
{
  if (isNodal())
    mooseError("ElementQualityAux only works on elemental fields.");
}

Real
ElementQualityAux::computeValue()
{
  return _current_elem->quality(_metric_type);
}
