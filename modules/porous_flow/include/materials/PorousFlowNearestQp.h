//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterial.h"

/**
 * Material designed to provide the nearest quadpoint to each node
 * in the element
 */
class PorousFlowNearestQp : public PorousFlowMaterial
{
public:
  static InputParameters validParams();

  PorousFlowNearestQp(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// The nearest quadpoint
  MaterialProperty<unsigned int> & _nearest_qp;
};
