//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

#include "libmesh/enum_elem_quality.h"

/**
 * Calculates element quality for each element
 */
class ElementQualityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ElementQualityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The metric type to use
  ElemQuality _metric_type;
};
