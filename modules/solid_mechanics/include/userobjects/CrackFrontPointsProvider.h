//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

/**
 * Base class for crack front points provider
 */
class CrackFrontPointsProvider : public ElementUserObject
{
public:
  static InputParameters validParams();

  CrackFrontPointsProvider(const InputParameters & parameters, const bool uses_mesh = false);

  /** get a set of points along a crack front from a XFEM GeometricCutUserObject
   * @return A vector which contains all crack front points
   */
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int /*num_crack_front_points*/) const = 0;

  /** get a set of normal vectors along a crack front from a XFEM GeometricCutUserObject
   * @return A vector which contains all crack front normals
   */
  virtual const std::vector<RealVectorValue>
  getCrackPlaneNormals(unsigned int /*num_crack_front_points*/) const = 0;

  /**
   * Getter for if a cutter mesh is used in a derived class.
   * @return bool indicating if a cutter mesh is used in the derived class
   */
  bool usesMesh() const { return _uses_mesh; }

protected:
  /// bool to set if CrackFrontPointsProvider derived objects use a cutter mesh
  const bool _uses_mesh;
};
