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

class TriInterWrapperMesh;

/**
 * This class calculates the area of the triangular, edge, and corner subchannels for hexagonal fuel
 * assemblies
 */
class TriInterWrapperFlowAreaIC : public TriInterWrapperBaseIC
{
public:
  TriInterWrapperFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
