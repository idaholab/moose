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

#include "LayeredSideAverage.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

//Forward Declarations
class LayeredSideFluxAverage;

template<>
InputParameters validParams<LayeredSideFluxAverage>();

/**
 * This UserObject computes side averages of a flux storing partial sums for the specified number of intervals in a direction (x,y,z).
 */
class LayeredSideFluxAverage : public LayeredSideAverage
{
public:
  LayeredSideFluxAverage(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  std::string _diffusivity;
  MaterialProperty<Real> & _diffusion_coef;
};

#endif
