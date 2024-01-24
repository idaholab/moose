//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolygonConcentricCircleMeshGeneratorBase.h"

/**
 * This PolygonConcentricCircleMeshGenerator object is designed to mesh a polygon geometry with
 * optional rings centered inside.
 */
class PolygonConcentricCircleMeshGenerator : public PolygonConcentricCircleMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PolygonConcentricCircleMeshGenerator(const InputParameters & parameters);
};
