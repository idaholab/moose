//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 * Get all of the elements that are intersected by a plane
 */
class ElementsAlongPlane : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  ElementsAlongPlane(const InputParameters & parameters);

  virtual void initialize() override;

  /**
   * Find the elements
   */
  virtual void execute() override;

protected:
  /// Point in the plane
  const Point & _p0;

  /// Normal vector to the plane
  const Point & _normal;

  /// The elements that intersect the plane
  VectorPostprocessorValue & _elem_ids;
};
