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

#ifndef BADCACHINGPOINTSOURCE_H
#define BADCACHINGPOINTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class BadCachingPointSource;

template<>
InputParameters validParams<BadCachingPointSource>();

/**
 * This DiracKernel tries to cache points using the wrong IDs and
 * throws an error.
 */
class BadCachingPointSource : public DiracKernel
{
public:
  BadCachingPointSource(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
};

#endif //BADCACHINGPOINTSOURCE_H
