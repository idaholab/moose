//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TriInterWrapperBaseIC.h"

/**
 * Sets the wetted perimeter of the quadrilater inter-wrapper flow channel
 */
class TriInterWrapperWettedPerimIC : public TriInterWrapperBaseIC
{
public:
  TriInterWrapperWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
