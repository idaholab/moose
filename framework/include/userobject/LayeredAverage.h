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

#ifndef LAYEREDAVERAGE_H
#define LAYEREDAVERAGE_H

#include "LayeredIntegral.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

//Forward Declarations
class LayeredAverage;

template<>
InputParameters validParams<LayeredAverage>();

/**
 * This UserObject computes  averages of a variable storing partial sums for the specified number of intervals in a direction (x,y,z).c
 */
class LayeredAverage : public LayeredIntegral
{
public:
  LayeredAverage(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

protected:
  /// Value of the volume for each layer
  std::vector<Real> _layer_volumes;
};

#endif
