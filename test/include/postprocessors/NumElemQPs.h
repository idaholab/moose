//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NUMELEMQPS_H
#define NUMELEMQPS_H

// MOOSE includes
#include "ElementIntegralPostprocessor.h"

// Forward declerations
class NumElemQPs;

template <>
InputParameters validParams<NumElemQPs>();

/**
 * An object for testing that the specified quadrature order is used.  It
 * counts the number of quadrature points.
 */
class NumElemQPs : public ElementIntegralPostprocessor
{
public:
  NumElemQPs(const InputParameters & parameters);
  virtual ~NumElemQPs();
  virtual Real computeIntegral();
  virtual Real computeQpIntegral();
};

#endif // NUMELEMQPS_H
