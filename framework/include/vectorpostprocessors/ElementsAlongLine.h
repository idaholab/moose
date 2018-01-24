//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTSALONGLINE_H
#define ELEMENTSALONGLINE_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class ElementsAlongLine;

template <>
InputParameters validParams<ElementsAlongLine>();

/**
 * Get all of the elements that are intersected by a line
 */
class ElementsAlongLine : public GeneralVectorPostprocessor
{
public:
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

#endif
