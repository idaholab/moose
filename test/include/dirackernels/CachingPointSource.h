//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
