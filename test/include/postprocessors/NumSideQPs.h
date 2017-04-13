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

#ifndef NUMSIDEQPS_H
#define NUMSIDEQPS_H

// MOOSE includes
#include "SideIntegralPostprocessor.h"

// Forward declerations
class NumSideQPs;

template <>
InputParameters validParams<SideIntegralPostprocessor>();

/**
 * An object for testing that the specified quadrature order is used.  It
 * counts the number of quadrature points.
 */
class NumSideQPs : public SideIntegralPostprocessor
{
public:
  NumSideQPs(const InputParameters & parameters);
  virtual ~NumSideQPs();
  virtual Real computeIntegral();
  virtual Real computeQpIntegral();
};

#endif // NUMSIDEQPS_H
