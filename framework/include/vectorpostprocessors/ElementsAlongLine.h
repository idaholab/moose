/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ELEMENTSALONGLINE_H
#define ELEMENTSALONGLINE_H

#include "GeneralVectorPostprocessor.h"

//Forward Declarations
class ElementsAlongLine;

template<>
InputParameters validParams<ElementsAlongLine>();

/**
 * Get all of the elements that are intersected by a line
 */
class ElementsAlongLine : public GeneralVectorPostprocessor
{
public:
  ElementsAlongLine(const std::string & name, InputParameters parameters);

  virtual ~ElementsAlongLine() {}

  /**
   * Clear it out.
   */
  virtual void initialize();

  /**
   * Find th elements
   */
  virtual void execute();

protected:
  /// The beginning of the line
  Point _start;

  /// The end of the line
  Point _end;

  /// The elements that intersect the line
  VectorPostprocessorValue & _elem_ids;
};

#endif
