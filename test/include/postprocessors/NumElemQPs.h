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

#ifndef NUMELEMQPS_H
#define NUMELEMQPS_H

// MOOSE includes
#include "ElementIntegralPostprocessor.h"

// Forward declerations
class NumElemQPs;

template <>
InputParameters validParams<ElementIntegralPostprocessor>();

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
