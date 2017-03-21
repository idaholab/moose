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

#ifndef CACHINGPOINTSOURCE_H
#define CACHINGPOINTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class CachingPointSource;

template <>
InputParameters validParams<CachingPointSource>();

/**
 * Adds a number of Dirac points with user-specified IDs to test
 * the Dirac point caching algorithm.
 */
class CachingPointSource : public DiracKernel
{
public:
  CachingPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
};

#endif // CACHINGPOINTSOURCE_H
