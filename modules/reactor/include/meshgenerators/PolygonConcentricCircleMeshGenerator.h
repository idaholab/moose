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
#include "MooseEnum.h"

class PolygonConcentricCircleMeshGenerator;

template <>
InputParameters validParams<PolygonConcentricCircleMeshGenerator>();

class PolygonConcentricCircleMeshGenerator : public PolygonConcentricCircleMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PolygonConcentricCircleMeshGenerator(const InputParameters & parameters);

protected:
  Real & _max_radius_meta;
};
