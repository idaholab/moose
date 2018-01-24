//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AVERAGEELEMENTSIZE_H
#define AVERAGEELEMENTSIZE_H

#include "ElementPostprocessor.h"

// Forward Declarations
class AverageElementSize;

template <>
InputParameters validParams<AverageElementSize>();

/**
 * This postprocessor computes an average element size (h) for the whole domain.
 */
class AverageElementSize : public ElementPostprocessor
{
public:
  AverageElementSize(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _total_size;
  int _elems;
};

#endif // AVERAGEELEMENTSIZE_H
