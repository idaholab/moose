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
 * Get all of the elements that are intersected by a line
 */
class ElementsAlongLine : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  ElementsAlongLine(const InputParameters & parameters);

  virtual void initialize() override;

  /**
   * Find the elements
   */
  virtual void execute() override;

protected:
  /// The beginning of the line
  Point _start;

  /// The end of the line
  Point _end;

  /// The elements that intersect the line
  VectorPostprocessorValue & _elem_ids;
};
