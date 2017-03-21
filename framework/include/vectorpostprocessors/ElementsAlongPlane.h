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

#ifndef ELEMENTSALONGPLANE_H
#define ELEMENTSALONGPLANE_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class ElementsAlongPlane;

template <>
InputParameters validParams<ElementsAlongPlane>();

/**
 * Get all of the elements that are intersected by a plane
 */
class ElementsAlongPlane : public GeneralVectorPostprocessor
{
public:
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

#endif
