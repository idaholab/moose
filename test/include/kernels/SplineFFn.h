//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPLINEFFN_H
#define SPLINEFFN_H

#include "Kernel.h"
#include "SplineFunction.h"

class SplineFFn;

template <>
InputParameters validParams<SplineFFn>();

/**
 * Forcing function defined with a spline
 */
class SplineFFn : public Kernel
{
public:
  SplineFFn(const InputParameters & parameters);
  virtual ~SplineFFn();

protected:
  virtual Real computeQpResidual();

  SplineFunction & _fn;
};

#endif /* SPLINEFFN_H */
