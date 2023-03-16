//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NearestPointBase.h"
#include "ElementAverageValue.h"
#include "ElementVariableVectorPostprocessor.h"

/**
 * Given a list of points this object computes the average of a variable
 * over the points closest to each provided point.
 */
class NearestPointAverage
  : public NearestPointBase<ElementAverageValue, ElementVariableVectorPostprocessor>
{
public:
  static InputParameters validParams();

  NearestPointAverage(const InputParameters & parameters);

  virtual Real spatialValue(const Point & point) const override;

  Real userObjectValue(unsigned int i) const;

  unsigned int nearestPointIndex(const Point & point) const;

  virtual void finalize() override;

protected:
  /// Postprocessor values for each point
  VectorPostprocessorValue & _np_post_processor_values;
};
