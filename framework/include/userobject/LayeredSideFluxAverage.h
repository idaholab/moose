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

#ifndef LAYEREDSIDEFLUXAVERAGE_H
#define LAYEREDSIDEFLUXAVERAGE_H

// MOOSE includes
#include "LayeredSideAverage.h"

// Forward Declarations
class LayeredSideFluxAverage;

template <>
InputParameters validParams<LayeredSideFluxAverage>();

/**
 * This UserObject computes side averages of a flux storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 */
class LayeredSideFluxAverage : public LayeredSideAverage
{
public:
  LayeredSideFluxAverage(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  std::string _diffusivity;
  const MaterialProperty<Real> & _diffusion_coef;
};

#endif
