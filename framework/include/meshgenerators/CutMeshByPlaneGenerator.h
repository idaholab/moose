//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "CutMeshByLevelSetGeneratorBase.h"

/**
 * This CutMeshByPlaneGenerator object is designed to trim the input mesh by removing all the
 * elements on one side of a given plane with special processing on the elements crossed by the
 * cutting plane to ensure a smooth cross-section. The output mesh only consists of TET4 elements.
 */
class CutMeshByPlaneGenerator : public CutMeshByLevelSetGeneratorBase
{
public:
  static InputParameters validParams();

  CutMeshByPlaneGenerator(const InputParameters & parameters);

protected:
  /// A point on the cutting plane
  const Point _plane_point;
  /// A normal vector of the cutting plane
  const Point _plane_normal;
};
