//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * Generates a polyline (open ended or looped) of Edge elements
 * through a series of nodal locations and other input parameters.
 */
class PolyLineMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  PolyLineMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::vector<Point> _points;

  bool _loop;

  boundary_id_type _bcid0, _bcid1;

  unsigned int _nebp;
};
