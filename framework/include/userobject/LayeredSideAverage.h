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

#ifndef LAYEREDSIDEAVERAGE_H
#define LAYEREDSIDEAVERAGE_H

// MOOSE includes
#include "LayeredSideIntegral.h"

// Forward Declarations
class LayeredSideAverage;

template <>
InputParameters validParams<LayeredSideAverage>();

/**
 * This UserObject computes side averages of a variable storing partial sums for the specified
 * number of intervals in a direction (x,y,z).c
 */
class LayeredSideAverage : public LayeredSideIntegral
{
public:
  LayeredSideAverage(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Value of the volume for each layer
  std::vector<Real> _layer_volumes;
};

#endif
