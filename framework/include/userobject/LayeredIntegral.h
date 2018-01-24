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

#ifndef LAYEREDINTEGRAL_H
#define LAYEREDINTEGRAL_H

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"
#include "LayeredBase.h"

// Forward Declarations
class LayeredIntegral;

template <>
InputParameters validParams<LayeredIntegral>();

/**
 * This UserObject computes volume integrals of a variable storing partial sums for the specified
 * number of intervals in a direction (x,y,z).c
 */
class LayeredIntegral : public ElementIntegralVariableUserObject, public LayeredBase
{
public:
  LayeredIntegral(const InputParameters & parameters);

  /**
   * Given a Point return the integral value associated with the layer that point falls in.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const override { return integralValue(p); }

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;
};

#endif
